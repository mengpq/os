
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
#include "file.h"

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	clear_screen();

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		init_task(p_proc,p_task,selector_ldt,p_task_stack);
		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}
	proc_table[0].ticks = proc_table[0].pre_ticks=proc_table[0].priority =   20;  proc_table[0].status = RUNNING;
	proc_table[1].ticks = proc_table[1].pre_ticks=proc_table[1].priority =   4;  proc_table[1].status = STOPPED;
	proc_table[2].ticks = proc_table[2].pre_ticks=proc_table[2].priority =   4;  proc_table[2].status = STOPPED;
	proc_table[3].ticks = proc_table[3].pre_ticks=proc_table[3].priority =   4;  proc_table[3].status = STOPPED;
	proc_table[4].ticks = proc_table[4].pre_ticks=proc_table[4].priority =   4;  proc_table[4].status = STOPPED;

	control_ticks();
	
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
                         control_ticks
 *======================================================================*/
void control_ticks(){
	int sum,i;
	sum=0;
	for (i=1; i<NR_TASKS; i++){
		if (proc_table[i].status==RUNNING) sum+=proc_table[i].priority;
	}
	if (!sum) return;
	//disp_int(sum); display_string(" ");
	for (i=1; i<NR_TASKS; i++){
		if (proc_table[i].status==RUNNING){
			int temp=20*proc_table[i].priority/sum;
			if (temp==0) temp=1;
			proc_table[i].pre_ticks=temp;
			//display_int(temp); display_string(" ");
		}
	}
	//display_string("\n");
}

/*======================================================================*
                         init_task
 *======================================================================*/
PUBLIC void init_task(PROCESS* p_proc, TASK* p_task, u16 selector_ldt, char* p_task_stack){
	strcpy(p_proc->p_name,p_task->name);
	p_proc->ldt_sel=selector_ldt;

	memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
	p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
	memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
	p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
	p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; 
	p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;

	//获取进程在内存的位置
	p_proc->regs.eip = (u32)p_task->initial_eip;
	//初始化栈的位置
	p_proc->regs.esp = (u32)p_task_stack;
	//初始化标志寄存器
	p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

	/*
	display_int(p_proc->regs.cs); display_string(" ");
	display_int(p_proc->regs.ds); display_string(" ");
	display_int(p_proc->regs.es); display_string(" ");
	display_int(p_proc->regs.ss); display_string(" ");
	display_int(p_proc->regs.gs); display_string(" ");
	display_int(p_proc->regs.eip); display_string(" ");
	display_int(p_proc->regs.esp); display_string(" ");
	display_int(p_proc->regs.eflags); display_string(" ");
	display_string("\n");
	*/

	p_proc->status = RUNNING;
}

void restart_process(char *name){
	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		if (strcmp(p_proc->p_name,name)==0){
			if (p_proc->status==SLEEPING){
				p_proc->status=RUNNING;
			} else{
				init_task(p_proc,p_task,selector_ldt,p_task_stack);
			}
			break;
		}
		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}
	control_ticks();
}


/*======================================================================*
                         process_command 
 *======================================================================*/

void display_string(char *st){
	disp_str(st);
	trace_cursor();
}

void display_int(int value){
	disp_int(value);
	trace_cursor();
}

void clear_screen(){
	int i=0;
	disp_pos = 0;
	for (i=0; i<4000; i++) display_string(" ");
	disp_pos = 0;
	trace_cursor();
}

int run(char* process_name){
	int i;
	for (i=1; i<NR_TASKS; i++) if (strcmp(process_name,proc_table[i].p_name)==0){
		if (proc_table[i].status==RUNNING) return -1;
		restart_process(process_name);
		//proc_table[i].status=RUNNING;
		return i;
	}
	return -1;
}

int kill(char* process_name){
	int i;
	for (i=1; i<NR_TASKS; i++) if (strcmp(process_name,proc_table[i].p_name)==0){
		if (proc_table[i].status!=RUNNING && proc_table[i].status!=SLEEPING) return -1;
		proc_table[i].status=STOPPED;
		control_ticks();
		return i;
	}
	return -1;
}

int sleep(char* process_name){
	int i;
	for (i=1; i<NR_TASKS; i++) if (strcmp(process_name,proc_table[i].p_name)==0){
		proc_table[i].status=SLEEPING;
		control_ticks();
		return i;
	}
}

int setp(char *process_name, int priority){
	int i=0;
	for (i=1; i<NR_TASKS; i++) if (strcmp(process_name,proc_table[i].p_name)==0){
		proc_table[i].priority=priority;
		control_ticks();
		return i;
	}
	return -1;
}

void display_byte_hex(u8 c){
	char output[2];
	u8 low=c&0xF,high=c>>4;
	memset(output,0,sizeof(output));
	if (high<=9) output[0]=high+'0'; else output[0]=high-10+'A';
	display_string(output);
	if (low<=9) output[0]=low+'0'; else output[0]=low-10+'A';
	display_string(output);
}

int dump_mem(char *start, char *len){
	if (!is_hex(start) || !is_number(len)) return -1;
	int st=hextoi(start),ed=st+atoi(len),i,total;
	char output[2];
	total=0;
	memset(output,0,sizeof(output));
	for (i=st; i<ed; i++){
		u8 temp=read_mem_byte(i);
		display_byte_hex(temp); 
		++total;
		if (total%8==0 && total%16!=0) display_string("-"); else display_string(" ");
		if ((total)%16==0) display_string("\n");
	}
	if (total%16!=0) display_string("\n");
	return 0;
}

void display_task(PROCESS* task){
	//display_string("pid = "); display_int(task->pid); 
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
	display_string("    clear             ---clear the screen\n");
	display_string("    run <name>        ---run the process by name\n");
	display_string("    sleep <name>      ---make the process sleep\n");
	display_string("    kill <name>       ---kill the process by name\n");
	display_string("    show -a -r -d -s  ---show all tasks's status\n");
	display_string("    help              ---show help information\n");
	display_string("    edit <name>       ---edit a file\n");
	display_string("    setp <name> <num> ---set the priority of process\n");
	display_string("    dump <addr> <len> ---view the memory [addr+len)\n");
}

void ls(){
	/* in file.c */
	show_all_fileinfo();
}

int rm(char *filename){
	return remove(filename);
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
			display_string("run successful");
			//display_int(pid);
			display_string("\n");
		}
	} else if (strcmp(CMD[0],"sleep")==0){
		if (total<2 || sleep(CMD[1])==-1){
		} else{
			display_string("the program: "); 
			display_string(CMD[1]);
			display_string(" is sleeping now!\n");
		}
	} else if (strcmp(CMD[0],"kill")==0){
		if (total<2 || kill(CMD[1])==-1){
			display_string("this program is neither running nor exists\n");
		} else{
			display_string("the program: ");
			display_string(CMD[1]);
			display_string(" is stooped now!\n");
		}
	} else if (strcmp(CMD[0],"setp")==0){
		if (total<3 || !is_number(CMD[2])){
			display_string("Usage: setp <name> <num>\n");
		} else if (setp(CMD[1],atoi(CMD[2]))==-1){
			display_string("i can't set the priority of ");
			display_string(CMD[1]);
			display_string("\n");
		} else {
			display_string("set priority successful\n");
		}
	} else if (strcmp(CMD[0],"show")==0){
		if (show_tasks(CMD[1])==-1){
			display_string("I can't recognize the parameter\n");
		}
	} else if (strcmp(CMD[0],"ls")==0){
		ls();
	} else if (strcmp(CMD[0],"rm")==0){
		if (total<2){
			display_string("Usage: rm <filename>\n");
		} else{
			if (rm(CMD[1])==-1){
				display_string(CMD[1]); 
				display_string(" no exist!\n");
			}
		}
	} else if (strcmp(CMD[0],"help")==0){
		show_help();
	} else if (strcmp(CMD[0],"edit")==0){
		start_editor(CMD[1]);
	} else if (strcmp(CMD[0],"dump")==0){
		if (dump_mem(CMD[1],CMD[2])==-1){
			display_string("I can't recognize the parameter\n");
		}
	} else{
		display_string(cmd); display_string(": ");
		display_string("no such command\n");
	}
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int temp=(task_f) PROCESSA;
	display_int(temp);
	int i;
	FILEINFO file;
	/*
	for (i=0; i<16384; i++){
		write_fileinfo(file);
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
