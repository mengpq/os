#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"

#define NORMAL 0
#define INSERT 1
#define BACKSPACE 0x0E
#define ENTERKEY 0x1C
#define NONEKEY 0xFF
#define ESC 0x01

struct point{
	int x,y;
} editor_pos,temp_pos;

u8 page_cache[4096];
int editor_mode,finish;

void editor_display(u8 c){
	char output[2]={0};
	output[0]=c;
	disp_pos=(editor_pos.x*80+editor_pos.y)*2;
	if (c!='\n') page_cache[disp_pos]=c;
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

void tracert_editorPos(){
	disp_pos=(editor_pos.x*80+editor_pos.y)*2;
	in_process(0);
}

void editor_clear_line(int line){
	int i;
	editor_pos.x=line; editor_pos.y=0;
	for (i=0; i<80; i++) editor_display(' ');
}

void editor_process_backspace(){
	if (editor_pos.y==0){
		if (editor_pos.x==0) return;
		editor_pos.y=79;
		--editor_pos.x;
	} else --editor_pos.y;
	editor_display(' ');
	--editor_pos.y;
	tracert_editorPos();
}

/* make line+1 become line */
void editor_down_line(int line){
	editor_pos.x=line;
	int i,j,pos=22*80*2;
	for (i=22; i>line; i--){
		for (j=0; j<80; j++){
			page_cache[pos]=page_cache[pos-160];
			pos+=2;
		}
		pos-=320;
	}
	editor_clear_line(line);
	editor_pos.x=line+1; editor_pos.y=0; pos=(line+1)*80*2;
	disp_int(pos);
	for (i=line+1; i<23; i++){
		for (j=0; j<80; j++){
			editor_display(page_cache[pos]);
			pos+=2;
		}
	}
}

void delete_one_row(){
	int x,y,pos=editor_pos.x*80*2;
	for (x=editor_pos.x; x<22; x++){
		for (y=0; y<80; y++){
			page_cache[pos]=page_cache[pos+160];
			pos+=2;
		}
	}
	for (y=0; y<80; y++){
		page_cache[pos]=' ';
		pos+=2;
	}
	pos=editor_pos.x*80*2;
	editor_pos.y=0;
	for (x=editor_pos.x; x<23; x++){
		for (y=0; y<80; y++){
			if (page_cache[pos]==0) page_cache[pos]=' ';
			editor_display(page_cache[pos]);
			pos+=2;
		}
	}
}

/* process command line */
void editor_process_command(){
	editor_pos.x=24; editor_pos.y=0;
	editor_display(':');
	int len=0;
	u8 scan_code,cmd[128];
	u8 *CMD[8];
	struct point temp;
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
}

/* process keyboard info */
void editor_process(u8 scan_code){
	struct point temp;
	if (scan_code == ENTERKEY){
		if (editor_pos.x==22) return;
		temp=editor_pos;
		editor_down_line(editor_pos.x+1);
		editor_pos=temp;
		editor_display('\n');
		++editor_pos.x; editor_pos.y=0;
		tracert_editorPos();
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
				if (editor_pos.x==22) return;
				++editor_pos.x;
			} else if (c=='h'){
				if (editor_pos.y==0) return;
				--editor_pos.y;
			} else if (c=='l' || c==' '){
				if (editor_pos.y==79) return;
				++editor_pos.y;
			} else if (c=='i'){
				temp_pos=editor_pos;
				editor_pos.x=23; editor_pos.y=0;
				editor_display_str("insert mode");
				editor_pos=temp_pos;
				editor_mode=INSERT;
			} else if (c=='d'){
				temp_pos=editor_pos;
				delete_one_row();
				editor_pos=temp_pos;
			} else if (c=='m'){
				editor_pos.y=35;
			} else if (c==';'){
				temp_pos=editor_pos;
				editor_process_command();
				editor_pos=temp_pos;
			} else if (c=='q'){
				finish=1;
			}
			tracert_editorPos();
		} else{
			if (scan_code==ESC){
				editor_mode=NORMAL;
				temp_pos=editor_pos;
				editor_pos.x=23; editor_pos.y=0;
				editor_display_str("normal mode");
				editor_pos=temp_pos;
			} else editor_display(c);
		}
		tracert_editorPos();
	}
}

void start_editor(char* filename){
	clear_screen();
	
	editor_mode=NORMAL;
	finish=0;

	editor_pos.x=23,editor_pos.y=0;
	memset(page_cache,4096,0);
	int cacheMemoryAddr=new_out(1);
	int i,j,k,len=strlen(filename);
	for (i=23,j=0; j<80-len; j++) editor_display('-');
	for (k=0; j<80; j++,k++) editor_display(filename[k]);
	editor_pos.x=23,editor_pos.y=0;
	editor_display_str("normal mode");
	editor_pos.x=0,editor_pos.y=0;
	tracert_editorPos();
	while (!finish){
		u8 scan_code=get_key_from_cache();
		if (scan_code==NONEKEY) continue;
		editor_process(scan_code);
	}
	clear_screen();
}
