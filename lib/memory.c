#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

u8 read_memory(int pos){
	return read_mem(pos);
}

void write_memory(int pos, u8 c){
	write_mem(pos,c);
}

int new_in(int total_block){
}

/* 在11MB以外找一段连续total_block个快 */
int new_out(int total_block){
	int i,j,count=0;
	for (i=FILEENTRY; i<MEMORYLIMIT; i+=BLOCKSIZE){
		u8 temp=read_memory(i);
		if (temp==0) ++count;
		if (count==total_block){
			return i-BLOCKSIZE*(count-1);
		}
	}
	return -1;
}
