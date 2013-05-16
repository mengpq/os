/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            keyboard.c
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
#include "keyboard.h"
#include "keymap.h"

PRIVATE  KB_INPUT	kb_in; 

/*======================================================================*
                            keyboard_handler
 *======================================================================*/
PUBLIC void keyboard_handler(int irq)
{
	u8 scan_code = in_byte(KB_DATA);

	if (kb_in.count < KB_IN_BYTES) {
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
			kb_in.p_head = kb_in.buf;
		}
		kb_in.count++;
	}
}


/*======================================================================*
                           init_keyboard
*======================================================================*/
PUBLIC void init_keyboard()
{
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;
	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);/*设定键盘中断处理程序*/
	enable_irq(KEYBOARD_IRQ);                       /*开键盘中断*/
}


/* 从键盘缓冲区获取一个处于按下状态的键的scan_code */
PUBLIC u8 get_key_from_cache(){
	if (kb_in.count==0) return -1;
	disable_int();
	u8 scan_code = *(kb_in.p_tail);
	kb_in.p_tail++;
	if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count--;
	enable_int();

	if (scan_code == 0xE1) {
		return -1;
	} else if (scan_code == 0xE0) {
		return -1;
	} else if (scan_code == 0x2A){
		return -1;
	} else if (scan_code == 0x36){
		return -1;
	} else if (scan_code == 0x3A){
	} else {
		/* 首先判断Make Code 还是 Break Code */
		int make = (scan_code & FLAG_BREAK ? FALSE : TRUE);
		if (!make) return -1;
		return scan_code;
	}
	return -1;
}

/* 获取scan_code对应的字符 */
PUBLIC u8 get_keymap(u8 scan_code){
	return keymap[(scan_code&0x7F)*MAP_COLS];
}

/*======================================================================*
                           keyboard_read
*======================================================================*/
PUBLIC void keyboard_read()
{
	u8	scan_code;
	char	output[2],cmd[256];
	int	make,enter_key,cmd_len,column;	/* TRUE: make;  FALSE: break. */
	display_string("acfast# ");
	memset(output, 0, 2);
	memset(cmd,0,256);
	column = enter_key = cmd_len = 0;

	while (!enter_key){
		scan_code = get_key_from_cache();
		if (scan_code==0xFF) continue;
		if (scan_code == 0x0E && disp_pos%160 == 8*2) continue;
		output[0] = get_keymap(scan_code);
		if (scan_code == 0x1C) output[0] = '\n';
		//backspace
		if (scan_code == 0x0E){
			disp_pos -= 2;
			output[0] = ' ';
			if (cmd_len > 0){
				cmd[--cmd_len] = 0;
			}
		} else if (scan_code != 0x1C){
			cmd[cmd_len++] = output[0];
		}

		//display key
		disp_str(output);

		//enter_key
		if (scan_code == 0x1C){
			enter_key = 1;
		}

		//backspace
		if (scan_code == 0x0E){
			disp_pos -= 2;
		}

		in_process(0);
	}
	process_command(cmd);
}
