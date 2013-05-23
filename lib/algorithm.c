#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC void split_by_space(char *CMD[], char *cmd, int *total){
	int i=0,firstChar=1;
	*total=0;
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

PUBLIC void tolower(char *st, char *ed){
	char *p=st;
	for (; p<ed; p++){
		if ('A'<=*p && *p<='Z') *p=*p-'A'+'a';
	}
}

PUBLIC int is_digit(char ch){
	return '0'<=ch && ch<='9';
}

PUBLIC int is_alpha(char ch){
	return ('a'<=ch && ch<='z') || ('A'<=ch && ch<='Z');
}

PUBLIC int is_number(char *st){
	int i,len;
	len=strlen(st);
	if (!len) return 0;
	for (i=0; i<len; i++) if (!('0'<=st[i] && st[i]<='9')) return 0;
	return 1;
}

PUBLIC int is_hex(char *st){
	int len=strlen(st);
	if (len<3) return 0;
	char temp[64];
	memcpy(temp,st,len+1);
	tolower(temp,temp+len);
	if (temp[0]!='0' || temp[1]!='x') return 0;
	int i;
	for (i=2; i<len; i++){
		if (!is_digit(temp[i]) && !is_alpha(temp[i])) return 0;
		if (is_alpha(temp[i]) && !('a'<=temp[i] && temp[i]<='f')) return 0;
	}
	return 1;
}
