os
==
os homework base on orange's code

* 文件
	* include/proc.h - 存放全局函数名 <br>
	* kernel/global.h - 存放全局变量
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
