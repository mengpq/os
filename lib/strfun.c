#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC int strlen(const char *str){
	int i=0;
	while (str[i]!=0) ++i;
	return i;
}

PUBLIC int strcmp(const char* a, const char* b){
	int i = 0;
	while (a[i] && b[i] && a[i]==b[i]) ++i;
	if (!a[i] && b[i]) return -1;
	if (a[i] && !b[i]) return 1;
	if (!a[i] && !b[i]) return 0;
	if (a[i]<b[i]) return -1;
	if (a[i]>b[i]) return 1;
}
