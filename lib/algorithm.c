#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC void split_by_space(char *CMD[], char *cmd, int *total){
	int i=0,firstChar=1;
	while (cmd[i]==' ') ++i;
	if (cmd[i]==0) return;
	while (cmd[i]!=0){
		if (cmd[i]!=' '){
			if (firstChar){
				CMD[(*total)++]=cmd+i;
				firstChar=0;
			}
		} else{
			cmd[i]=0;
			firstChar=1;
		}
		++i;
	}
}
