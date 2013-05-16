
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void process_command(char* cmd);
void clear_screen();
void display_string();
void display_int();
void TestA();
void TestB();
void TestC();
void TestD();

/* strfunc.c */
PUBLIC int strlen(char* str);

/* editor.c */
PUBLIC void start_editor(char *filename);

/* memory.c */
PUBLIC u8 read_memory(int pos);
PUBLIC void write_memory(int pos, u8 c);
PUBLIC int new_in(int total_block);
PUBLIC int new_out(int total_block);

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();
PUBLIC u8 get_key_from_cache();
PUBLIC u8 get_keymap(u8 scan_code);

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(u32 key);

/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */

/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();

