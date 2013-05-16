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

struct point{
	int x,y;
} editor_pos,temp_pos;

u8 keyboard_cache[128];
u8 page_cache[4096];
int editor_mode,finish;

void display(u8 c){
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

void display_str(char *st){
	int i=0;
	while (st[i]){
		display(st[i]);
		++i;
	}
}

void tracert_editorPos(){
	disp_pos=(editor_pos.x*80+editor_pos.y)*2;
	in_process(0);
}

void process_backspace(){
	if (editor_pos.y==0){
		if (editor_pos.x==0) return;
		editor_pos.y=79;
		--editor_pos.x;
	} else --editor_pos.y;
	display(' ');
	--editor_pos.y;
	tracert_editorPos();
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
			display(page_cache[pos]);
			pos+=2;
		}
	}
}

void process(u8 scan_code){
	if (scan_code == 0x1C){
		if (editor_pos.x==22) return;
		display('\n');
		++editor_pos.x; editor_pos.y=0;
		tracert_editorPos();
	} else
	if (scan_code==0x0E){
		process_backspace();
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
				display_str("insert mode");
				editor_pos=temp_pos;
				editor_mode=INSERT;
			} else if (c=='d'){
				temp_pos=editor_pos;
				delete_one_row();
				editor_pos=temp_pos;
			} else if (c=='m'){
				editor_pos.y=40;
			} else if (c=='q'){
				finish=1;
			}
			tracert_editorPos();
		} else{
			if (scan_code==0x01){
				editor_mode=NORMAL;
				temp_pos=editor_pos;
				editor_pos.x=23; editor_pos.y=0;
				display_str("normal mode");
				editor_pos=temp_pos;
			} else display(c);
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
	for (i=23,j=0; j<80-len; j++) display('-');
	for (k=0; j<80; j++,k++) display(filename[k]);
	editor_pos.x=23,editor_pos.y=0;
	display_str("normal mode");
	editor_pos.x=0,editor_pos.y=0;
	tracert_editorPos();
	while (!finish){
		u8 scan_code=get_key_from_cache();
		if (scan_code==0xFF) continue;
		process(scan_code);
	}
	clear_screen();
}
