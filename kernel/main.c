
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	//display_string("-----\"kernel_main\" begins-----\n");
	clear_screen();

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK; p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority =  10;  proc_table[0].status = RUNNING;
	proc_table[1].ticks = proc_table[1].priority =   50;  proc_table[1].status = STOPPED;
	proc_table[2].ticks = proc_table[2].priority =   5;  proc_table[2].status = STOPPED;
	proc_table[3].ticks = proc_table[3].priority =   5;  proc_table[3].status = STOPPED;
	proc_table[4].ticks = proc_table[4].priority =   5;  proc_table[4].status = STOPPED;

	//for (i=0; i<NR_TASKS; i++) proc_table_bak[i]=proc_table[i];

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
    init_keyboard();

	restart();

	while(1){}
}


/*======================================================================*
                         process_command 
 *======================================================================*/

void display_string(char *st){
	disp_str(st);
	in_process(0);
}

void display_int(int value){
	disp_int(value);
	in_process(0);
}

int atoi(char *str){
	int ret=0,i=0;
	while (str[i]!=0 && '0'<=str[i] && str[i]<='9'){
		ret=ret*10+(str[i]-'0');
		++i;
	}
	return ret;
}

void clear_screen(){
	int i=0;
	disp_pos = 0;
	for (i=0; i<4000; i++) display_string(" ");
	disp_pos = 0;
	in_process(0);
}

int strcmp(const char* a, const char* b){
	int i = 0;
	while (a[i] && b[i] && a[i]==b[i]) ++i;
	if (!a[i] && b[i]) return -1;
	if (a[i] && !b[i]) return 1;
	if (!a[i] && !b[i]) return 0;
	if (a[i]<b[i]) return -1;
	if (a[i]>b[i]) return 1;
}

void split_by_space(char *CMD[], char *cmd, int *total){
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

int run(char* process_name){
	int i;
	for (i=0; i<NR_TASKS; i++) if (strcmp(process_name,proc_table[i].p_name)==0){
		if (proc_table[i].status==RUNNING) return -1;
		//proc_table[i]=proc_table_bak[i];
		proc_table[i].status=RUNNING;
		return i;
	}
	return -1;
}

int kill(char* process_name){
	int i;
	for (i=0; i<NR_TASKS; i++) if (strcmp(process_name,proc_table[i].p_name)==0){
		if (proc_table[i].status!=RUNNING) return -1;
		proc_table[i].status=STOPPED;
		return i;
	}
	return -1;
}

void display_task(PROCESS* task){
	display_string("pid = "); display_int(task->pid); 
	display_string("  name = "); display_string(task->p_name);
	display_string("  status = "); 
	int status=task->status;
	if (status == STOPPED) display_string("STOPPED\n"); else
	if (status == RUNNING) display_string("RUNNING\n"); else
	if (status == SLEEPING) display_string("SLEEPING\n");
}

int show_tasks(char *argc){
	int i,total=0;
	if (strcmp(argc,"-a")==0){
		display_string("There are "); display_int(NR_TASKS-1); display_string(" task!\n");
		for (i=1; i<NR_TASKS; i++) display_task(proc_table+i);
		return 0;
	} else if (strcmp(argc,"-r")==0){
		for (i=1; i<NR_TASKS; i++) if (proc_table[i].status==RUNNING) ++total;
		display_string("There are "); display_int(total); display_string(" task RUNNING!\n");
		for (i=1; i<NR_TASKS; i++) if (proc_table[i].status==RUNNING) display_task(proc_table+i);
		return 0;
	} else if (strcmp(argc,"-d")==0){
		for (i=1; i<NR_TASKS; i++) if (proc_table[i].status==STOPPED) ++total;
		display_string("There are "); display_int(total); display_string(" task STOPPED!\n");
		for (i=1; i<NR_TASKS; i++) if (proc_table[i].status==STOPPED) display_task(proc_table+i);
		return 0;
	} else if (strcmp(argc,"-s")==0){
		for (i=1; i<NR_TASKS; i++) if (proc_table[i].status==SLEEPING) ++total;
		display_string("There are "); display_int(total); display_string(" task SLEEPING!\n");
		for (i=1; i<NR_TASKS; i++) if (proc_table[i].status==SLEEPING) display_task(proc_table+i);
		return 0;
	} else {
		for (i=1; i<NR_TASKS; i++) if (strcmp(proc_table[i].p_name,argc)==0){
			display_task(proc_table+i);
			return 0;
		}
	}
	return -1;
}

void show_help(){
	display_string("You can type command below and case sensitive\n");
	display_string("    clear            ---clear the screen\n");
	display_string("    run <name>       ---run the process by name\n");
	display_string("    kill <name>      ---kill the process by name\n");
	display_string("    show -a -r -d -s ---show all tasks's status\n");
	display_string("    help             ---show help information\n");
}

void process_command(char* cmd){
	char* CMD[10];
	memset(CMD,0,10);
	int i,pid,total,status;
	total=0;
	split_by_space(CMD,cmd,&total);
	if (total==0) return;
	if (strcmp(CMD[0],"clear")==0){
		clear_screen();
	} else if (strcmp(CMD[0],"run")==0){
		if (total<2 || (pid=run(CMD[1]))==-1){
			display_string("this program is not exists or already running!\n");
		} else{
			display_string("run successful and the pid is ");
			display_int(pid);
			display_string("\n");
		}
	} else if (strcmp(CMD[0],"kill")==0){
		if (total<2 || kill(CMD[1])==-1){
			display_string("this program is neither running nor exists\n");
		} else{
			display_string("the program: ");
			display_string(CMD[1]);
			display_string(" is stooped now!\n");
		}
	} else if (strcmp(CMD[0],"show")==0){
		if (show_tasks(CMD[1])==-1){
			display_string("I can't recognize the parameter\n");
		}
	} else if (strcmp(CMD[0],"help")==0){
		show_help();
	} else if (strcmp(CMD[0],"editor")==0){
		start_editor("acfast.txt");
	} else{
		display_string("No such command\n");
	}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	/*
	int temp=FILEENTRY;
	u8 t;
	for (t=0; t<255; t++){
		write_memory(temp,t);
		temp+=BLOCKSIZE;
	}
	*/
	while (1) {
		//PROCESSA();
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int temp=new_out(10);
	disp_int(temp); display_string(" ");
	/*
	int temp=0x500000,i;
	int delta=0x500000/1024;
	for (i=0; i<delta; i++){
		u8 ret=read_memory(temp);
		//disp_int(ret); display_string(" ");
		temp+=1024;
	}
	display_string("scan 5MB memory\n");
	*/
	while(1){
		//PROCESSB();
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	while(1){
		PROCESSC();
	}
}

/*======================================================================*
                               TestD
 *======================================================================*/
void TestD()
{
	while(1){
		PROCESSD();
	}
}
