#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"
#include "file.h"

#define NORMAL 0
#define INSERT 1
#define BACKSPACE 0x0E
#define ENTERKEY 0x1C
#define NONEKEY 0xFF
#define MAXROW 22
#define MAXCOLUMN 79
#define ESC 0x01

struct point{
	int x,y;
} editor_pos;

u8 page_cache[4096];
char editor_filename[32];
char editor_linebuffer[256];
int editor_mode,finish,editor_changepos;

int calc_pos(int x, int y){
	return x*80+y;
}

void editor_display(u8 c){
	char output[2]={0};
	if (c==0) c=' ';
	output[0]=c;
	disp_pos=(editor_pos.x*80+editor_pos.y)*2;
	if (c!='\n') page_cache[calc_pos(editor_pos.x,editor_pos.y)]=c;
	display_string(output);

	if (++editor_pos.y==80){
		editor_pos.y=0;
		if (++editor_pos.x>=23) editor_pos.x=0;
	}
}

void editor_display_str(char *st){
	int i=0;
	while (st[i]){
		editor_display(st[i]);
		++i;
	}
}

void editor_goinsertmode(){
	editor_mode=INSERT;
	editor_pos.x=23; editor_pos.y=0;
	editor_display_str("insert mode");
}

void editor_gonormalmode(){
	editor_mode=NORMAL;
	editor_pos.x=23; editor_pos.y=0;
	editor_display_str("normal mode");
}

void editor_init(char *filename){
	clear_screen();
	finish=0;
	memset(editor_linebuffer,0,sizeof(editor_linebuffer));
	memset(editor_filename,0,sizeof(editor_filename));
	memcpy(editor_filename,filename,strlen(filename));
	editor_pos.x=23,editor_pos.y=0;
	int i,len=strlen(filename);
	for (i=0; i<80-len; i++) editor_display('-');
	editor_display_str(filename);
	editor_gonormalmode();
}

void editor_save_file(char *st){
	FILEINFO file;
	memset(&file,0,sizeof(file));
	if (get_file_info_by_name(st,&file)!=-1){
		//display_string("file exist\n");
	} else{
		//display_string("Find the file\n");
		memcpy(file.name,st,strlen(st));
		file.size=sizeof(page_cache);
		file.start_pos=new_space(FILESTOREADDR,file.size);
		write_fileinfo(file);
	}
	write_data(file.start_pos,page_cache,sizeof(page_cache));
}

void editor_open_file(char *st){
	FILEINFO file;
	memset(&file,0,sizeof(file));
	if (get_file_info_by_name(st,&file)!=-1){
		memcpy(page_cache,(void *)file.start_pos,sizeof(page_cache));
		editor_init(file.name);
		memcpy(editor_filename,file.name,sizeof(file.name));
		editor_pos.x=0,editor_pos.y=0;
		int i,j;
		for (i=0; i<23; i++) for (j=0; j<80; j++) editor_display(page_cache[calc_pos(i,j)]);
	}
}

void trace_editorpos(){
	disp_pos=(editor_pos.x*80+editor_pos.y)*2;
	trace_cursor();
}

void editor_clear_line(int line){
	int i;
	editor_pos.x=line; editor_pos.y=0;
	for (i=0; i<80; i++) editor_display(' ');
}

void editor_process_backspace(){
	if (editor_pos.y==0){
		if (editor_pos.x==0) return;
		editor_pos.y=MAXCOLUMN;
		--editor_pos.x;
	} else --editor_pos.y;
	editor_display(' ');
	--editor_pos.y;
	trace_editorpos();
}

void editor_line_search(int direction){
	int scan_code;
	u8 ch;
	while (TRUE){
		scan_code=get_key_from_cache();
		if (scan_code==NONEKEY) continue;
		if (scan_code==ESC) return;
		if (scan_code==ENTERKEY) return;
		ch=get_keymap(scan_code);
		break;
	}
	int x,y;
	if (direction==1){
		x=editor_pos.x,y=editor_pos.y+1;
		for (; y<80; y++) if (page_cache[calc_pos(x,y)]==ch){
			editor_pos.y=y;
			return;
		}
	} else{
		x=editor_pos.x,y=editor_pos.y-1;
		if (y<=0) return;
		for (; y>=0; y--) if (page_cache[calc_pos(x,y)]==ch){
			editor_pos.y=y;
			return;
		}
	}
}

void editor_replace_char(){
	int scan_code;
	while (TRUE){
		scan_code=get_key_from_cache();
		if (scan_code==NONEKEY) continue;
		if (scan_code==ESC) return;
		if (scan_code==ENTERKEY) return;
		char ch=get_keymap(scan_code);
		editor_display(ch);
		if (editor_pos.y==0){
			editor_pos.y=MAXCOLUMN;
			--editor_pos.x;
		} else --editor_pos.y;
		trace_editorpos();
		return;
	}
}

/* make current line down k line*/
void editor_down_line(int current, int k){
	int i,j,pos=current*80;
	for (i=MAXROW; i>=current+k; i--) memcpy(page_cache+i*80,page_cache+(i-k)*80,80);
	for (i=0; i<k; i++) memcpy(page_cache+(i+current)*80,editor_linebuffer,80);
	editor_pos.x=current; editor_pos.y=0;
	for (i=current; i<23; i++) for (j=0; j<80; j++){
		editor_display(page_cache[calc_pos(i,j)]);
	}
}

void delete_one_row(){
	int x,y,pos=editor_pos.x*80;
	for (x=editor_pos.x; x<MAXROW; x++){
		memcpy(page_cache+x*80,page_cache+(x+1)*80,80);
	}
	pos=MAXROW*80;
	for (y=0; y<80; y++){
		page_cache[pos]=' ';
		pos++;
	}
	pos=editor_pos.x*80;
	editor_pos.y=0;
	for (x=editor_pos.x; x<23; x++){
		for (y=0; y<80; y++){
			editor_display(page_cache[pos]);
			pos++;
		}
	}
}

/* process command line */
void editor_process_command(){
	struct point backup_pos=editor_pos;
	editor_pos.x=24; editor_pos.y=0;
	editor_display(':');
	int len=0,total=0;
	u8 scan_code,cmd[64];
	u8 *CMD[8];
	memset(cmd,0,sizeof(cmd));
	while (TRUE){
		scan_code=get_key_from_cache();
		if (scan_code==NONEKEY) continue;
		if (scan_code==BACKSPACE){
			editor_process_backspace();
			if (--len<0) return;
		} else
		if (scan_code==ESC){
			editor_clear_line(editor_pos.x);
			return;
		} else
		if (scan_code!=ENTERKEY){
			cmd[len++]=get_keymap(scan_code);
			editor_display(get_keymap(scan_code));
		} else{
			editor_clear_line(editor_pos.x);
			break;
		}
	}
	split_by_space(CMD,cmd,&total);
	editor_changepos=0;
	editor_pos=backup_pos;
	if (strcmp(CMD[0],"e")==0){
		if (total==1) return;
		editor_open_file(CMD[1]);
	} else if (strcmp(CMD[0],"w")==0){
		editor_save_file(editor_filename);
	} else if (strcmp(CMD[0],"q")==0){
		finish=1;
	} else if (is_number(CMD[0])){
		if (total==1) return;
		if (strcmp(CMD[1],"j")==0){
			editor_changepos=1;
			editor_pos.x+=atoi(CMD[0]);
			if (editor_pos.x>MAXROW) editor_pos.x=MAXROW;
		} else if (strcmp(CMD[1],"k")==0){
			editor_changepos=1;
			editor_pos.x-=atoi(CMD[0]);
			if (editor_pos.x<0) editor_pos.x=0;
		} else if (strcmp(CMD[1],"h")==0){
			editor_changepos=1;
			editor_pos.y-=atoi(CMD[0]);
			if (editor_pos.y<0) editor_pos.y=0;
		} else if (strcmp(CMD[1],"l")==0){
			editor_changepos=1;
			editor_pos.y+=atoi(CMD[0]);
			if (editor_pos.y>MAXCOLUMN) editor_pos.y=MAXCOLUMN;
		} else if (strcmp(CMD[1],"g")==0 || strcmp(CMD[1],"gg")==0){
			editor_changepos=1;
			editor_pos.x=atoi(CMD[0]);
			if (editor_pos.x>MAXROW) editor_pos.x=MAXROW;
		} else if (strcmp(CMD[1],"p")==0){
			editor_changepos=1;
			editor_down_line(editor_pos.x+1,atoi(CMD[0]));
			editor_pos=backup_pos;
			editor_pos.x+=atoi(CMD[0]);
			if (editor_pos.x>MAXROW) editor_pos.x=MAXROW;
		}
	} 
}

/* process keyboard info */
void editor_process(u8 scan_code){
	struct point backup_pos;
	if (scan_code == ENTERKEY){
		if (editor_pos.x==MAXROW) return;
		backup_pos=editor_pos;
		char data[160];
		memcpy(data,editor_linebuffer,160);
		memset(editor_linebuffer,0,160);
		editor_down_line(editor_pos.x+1,1);
		memcpy(editor_linebuffer,data,160);
		editor_pos=backup_pos;
		editor_display('\n');
		++editor_pos.x; editor_pos.y=0;
		trace_editorpos();
	} else
	if (scan_code==BACKSPACE){
		editor_process_backspace();
	} else{
		u8 c=get_keymap(scan_code);
		if (editor_mode==NORMAL){
			if (c=='k'){
				if (editor_pos.x==0) return;
				--editor_pos.x;
			} else if (c=='j'){
				if (editor_pos.x==MAXROW) return;
				++editor_pos.x;
			} else if (c=='h'){
				if (editor_pos.y==0) return;
				--editor_pos.y;
			} else if (c=='l' || c==' '){
				if (editor_pos.y==MAXCOLUMN) return;
				++editor_pos.y;
			} else if (c=='i'){
				backup_pos=editor_pos;
				editor_goinsertmode();
				editor_pos=backup_pos;
				editor_mode=INSERT;
			} else if (c=='d'){
				backup_pos=editor_pos;
				delete_one_row();
				editor_pos=backup_pos;
			} else if (c=='m'){
				editor_pos.y=35;
			} else if (c==';'){
				backup_pos=editor_pos;
				editor_process_command();
				if (!editor_changepos) editor_pos=backup_pos;
			} else if (c=='w'){
				editor_save_file(editor_filename);
			} else if (c=='e'){
				editor_open_file(editor_filename);
			} else if (c=='y'){
				memcpy(editor_linebuffer,page_cache+editor_pos.x*80,160);
			} else if (c=='p'){
				backup_pos=editor_pos;
				editor_down_line(editor_pos.x+1,1);
				editor_pos=backup_pos;
				editor_pos.x++;
				if (editor_pos.x>MAXROW) editor_pos.x=MAXROW;
			} else if (c=='r'){
				editor_replace_char();
			} else if (c=='f'){
				editor_line_search(1);
			} else if (c=='b'){
				editor_line_search(0);
			}else if (c=='q'){
				finish=1;
			}
			trace_editorpos();
		} else{
			if (scan_code==ESC){
				backup_pos=editor_pos;
				editor_gonormalmode();
				editor_pos=backup_pos;
			} else editor_display(c);
		}
		trace_editorpos();
	}
}

void start_editor(char* filename){
	memset(page_cache,0,sizeof(page_cache));
	editor_init(filename);
	editor_open_file(filename);
	int i,j,k;
	editor_pos.x=0,editor_pos.y=0;
	trace_editorpos();
	while (!finish){
		u8 scan_code=get_key_from_cache();
		if (scan_code==NONEKEY) continue;
		editor_process(scan_code);
	}
	clear_screen();
}

