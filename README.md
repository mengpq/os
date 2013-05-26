os
==
os homework base on orange's code

* 文件
	* kernel/
		* kernel.asm
			* _start - 加载gdt、idt、TSS，设置内核栈，跳至kernel_main
			* restart - 重启切换后的进程
		* start.c
			* cstart - 将LOADER中的GDT复制到新的GDT中
		* clock.c 
			* init_clock - 初始化8259A，设置时钟中断处理程序
			* clock_handler - 时钟中断处理，每产生一次时钟中断ticks就+1
			* milli_delay - 延时，10ms一个单位 
		* tty.c
			* task_tty - 监视键盘输入
			* trace_cursor - 跟踪光标
		* keyboard.c
			* init_keyboard - 初始化键盘中断，设置中断处理程序
			* keyboard_handler - 处理键盘中断，将输入的键盘编码加进键盘队列里面
			* get_key_from_cache - 取键盘队列队首元素(按键编码)
			* get_keymap - 获取键盘编码对应的键(字符)
			* keyboard_read - 处理一个命令行，输入回车表示命令输入结束
		* memory.c
			* is_free - 判断一段内存是否为未使用
			* mark - 标记一段内存已经使用
			* new_space - 从某个地址开始，申请一段长度大于等于size的连续内存
		* file.c
			* write_fileinfo - 将文件信息写到内存里面
			* get_file_info_by_name - 按照文件名查询文件文件信息(没有实现目录结构)
			* remove - 删除文件，清空文件占用的内存
			* write_data - 将文件数据写到内存
			* show_all_fileinfo - 输出所有文件的文件名
		* global.c
			* 存储全局变量

	* include/proc.h - 存放全局函数名 <br>
	* kernel/global.h - 存放全局变量
	* kernel
* 文件存储位置
	* #define MEMORYMAP              /* 1bit表示一个BLOCK是否被使用，BLOCK从10M开始算起*/
	* #define ROOTDIRECTORY 0xA00000 /* 文件目录项地址 10M */
	* #define FILESTOREADDR 0xB00000     /* 文件存储初始地址 11M */
	* #define MEMORYBLOCK 0x40     /* 每一个block的容量64byte */
	* #define MEMORYLIMIT 0x4000000  /* 最大内存，和bochsrc对应 64M*/

* editor
	* 实现vim简单的几个功能
	* 只操作一个页面2000左右个字符
	* normal mode key
		* m 将光标移动到中间列
		* d 删除当前行
		* q 退出编辑
		* i 进入插入模式
		* w 保存
		* ; 进入命令行模式
	* insert mode key
		* ESC 退出insert mode
	* command line mode
		* ESC 退出命令行模式
		* enter 完成命令输入
		* e <filename> 打开文件
		* w 保存
		* q 退出

* 根目录
	* 文件信息
		* 16字节的文件名，4字节的后缀，12字节作者信息，4字节的大小，4字节模式，4字节的起始位置

* algorithm
	* split_by_space(char *CMD[], char *cmd, int *total);
		* 将cmd存储的字符串按照空格隔开，CMD[]里面存储着各个分开的字符串的首字母，total是分开的字符串的个数，cmd的空格会被修改为\0
