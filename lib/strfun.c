#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC int strlen(char *str){
	int i=0;
	while (str[i]!=0) ++i;
	return i;
}
