#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC int new_in(int total_block){
}

PUBLIC int is_free(int st, int ed){
	st+=MEMORYMAP; ed+=MEMORYMAP;
	int i;
	for (i=st; i<ed; i++){
		u8 temp=read_mem_byte(i);
		if (temp){
			return 0;
		}
	}
	return 1;
}

PUBLIC void mark(int st, int ed, char value){
	st+=MEMORYMAP; ed+=MEMORYMAP;
	for (; st<ed; st++){
		write_mem_byte(st,value);
	}
}

PUBLIC int new_space(int start, int size){
	start=(start-MEMORYMAPLIMIT)/BLOCKSIZE;
	size=(size-1)/BLOCKSIZE+1;
	int i;
	for (i=start; i+size<MEMORYMAPLIMIT; i++){
		if (is_free(i,i+size)){
			mark(i,i+size,1);
			return MEMORYMAPLIMIT+i*BLOCKSIZE;
		}
	}
	/*
	int i;
	for (i=start; i+size<ROOTDIRECTORYLIMIT; i+=delta){
		if (is_free(i,i+size)) return i;
	}
	return -1;
	*/
}

/* 在11MB以外找一段连续total_block个快 */
PUBLIC int new_out(int total_block){
	/*
	int i,j,count=0;
	for (i=FILE; i<MEMORYLIMIT; i+=BLOCKSIZE){
		u8 temp=read_mem_byte(i);
		if (temp==0) ++count;
		if (count==total_block){
			return i-BLOCKSIZE*(count-1);
		}
	}
	*/
	return -1;
}
