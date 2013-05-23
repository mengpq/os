#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "file.h"
#include "keyboard.h"

PUBLIC void write_fileinfo(FILEINFO info){
	int size=sizeof(info),i;
	//apply for a free continuous space 
	int start=new_space(ROOTDIRECTORY,size);
	memcpy((void *)start,&info,sizeof(info));
}

int get_file_info_by_name(char *filename, FILEINFO *file){
	int i,addr;
	for (i=0,addr=MEMORYMAP; i<16384; i++,addr++){
		u8 temp=read_mem_byte(addr);
		if (temp){
			memcpy(file,(void *)(i*BLOCKSIZE+ROOTDIRECTORY),sizeof(*file));
			if (strcmp(file->name,filename)==0) return i;
		}
	}
	return -1;
}

PUBLIC int remove(char *filename){
	FILEINFO file;
	memset(&file,0,sizeof(file));
	int index=get_file_info_by_name(filename,&file);
	if (index==-1) return -1;
	memset((void *)(file.start_pos),0,file.size);
	memset((void *)(index*BLOCKSIZE+ROOTDIRECTORY),0,sizeof(file));
	memset((void *)(MEMORYMAP+index),0,1);
	return 0;
}

PUBLIC void read_fileinfo(FILEINFO info){
	/*
	FILEINFO file;
	int temp=ROOTDIRECTORY;
	memcpy(&file,(void*)ROOTDIRECTORY,40);
	display_string(file.name);
	display_string(" "); display_string(file.ext);
	display_string(" ");
	display_string(file.author);
	display_string(" ");
	display_int(file.size);
	display_string(" ");
	display_int(file.start_pos);
	display_string("\n");
	*/
}

PUBLIC void write_data(int start, char *buffer, int size){
	memcpy((void *)start, buffer,size);
}

PUBLIC void show_all_fileinfo(){
	int i,addr,total;
	FILEINFO file;
	total=0;
	for (i=0,addr=MEMORYMAP; i<16384; i++,addr++){
		u8 temp=read_mem_byte(addr);
		if (temp){
			++total;
			memcpy(&file,(void *)(i*BLOCKSIZE+ROOTDIRECTORY),sizeof(file));
			display_string(file.name); display_string("    ");
		}
	}
	if (total) display_string("\n");
}
