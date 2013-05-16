os
==
os homework base on orange's code

* 文件
	* include/proc.h - 存放全局函数名 <br>
	* kernel/global.h - 存放全局变量
* 文件存储位置
	* #define ROOTDIRECTORY 0xA00000 /* 文件目录项地址 10M */
	* #define FILEENTRY 0xB00000     /* 文件存储初始地址 11M */
	* #define MEMORYBLOCK 0x1000     /* 每一个block的容量4K */
	* #define MEMORYLIMIT 0x4000000  /* 最大内存，和bochsrc对应 64M*/

* editor
	* 实现vim简单的几个功能
	* 只操作一个页面2000左右个字符
