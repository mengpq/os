
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"
#include "file.h"

PUBLIC	PROCESS			proc_table[NR_TASKS], proc_table_bak[NR_TASKS];

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {{task_tty, STACK_SIZE_TTY, "tty"},
					{PROCESSA, STACK_SIZE_TESTA, "a"},
					{PROCESSB, STACK_SIZE_TESTB, "b"},
					{PROCESSC, STACK_SIZE_TESTC, "c"},
					{PROCESSD, STACK_SIZE_TESTD, "d"}};

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks};

