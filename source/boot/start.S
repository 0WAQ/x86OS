/**
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * 该部分程序存储于磁盘的第1个扇区，在计算机启动时将会由BIOS加载到0x7c00处
 * 之后，将由BIOS跳转至0x7c00处开始运行
 * 
 */
	#include "boot.h"

  	// 16位代码, 务必加上
  	.code16
 	.text
	.global _start
	.extern boot_entry

/**
 *
 * 在CMakeLists中设置:
 * 1. _start在最前面
 * 2. 将boot程序加载到0x7c00
 * 
 */
_start:

	// 重置数据段寄存器
	mov $0, %eax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	// 设置栈顶在0x7c00, 参考链接 https://wiki.osdev.org/Memory_Map_(x86)
	mov $_start, %esp

	// 显示boot加载完成提示
	mov $0xe, %ah
	mov $'.', %al
	int $0x10
	
	mov $0xe, %ah
	mov $'.', %al
	int $0x10

	mov $0xe, %ah
	mov $'.', %al
	int $0x10

	mov $0xe, %ah
	mov $'B', %al
	int $0x10

	mov $0xe, %ah
	mov $'o', %al
	int $0x10

	mov $0xe, %ah
	mov $'o', %al
	int $0x10

	mov $0xe, %ah
	mov $'t', %al
	int $0x10

	mov $0xe, %ah
	mov $'i', %al
	int $0x10
	
	mov $0xe, %ah
	mov $'n', %al
	int $0x10

	mov $0xe, %ah
	mov $'g', %al
	int $0x10

	mov $0xe, %ah
	mov $'.', %al
	int $0x10

	mov $0xe, %ah
	mov $'.', %al
	int $0x10

	mov $0xe, %ah
	mov $'.', %al
	int $0x10

	mov $0xe, %ah
	mov $'\n', %al
	int $0x10

	mov $0xe, %ah
	mov $'\r', %al
	int $0x10

// 加载loader, 只支持磁盘1
// https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)
read_loader:
	mov $0x8000, %bx	// 将loader读取到0x8000处
	mov $0x02, %cx		// ch: 磁道号, cl起始扇区号
	mov $0x02, %ah		// ah: 0x02读磁盘命令
	mov $64,   %al		// al: 读取的扇区数量, 必须小于128, 暂设置成32KB
	mov $0x0080, %dx	// dh: 磁头号, dl: 驱动器号0x80(磁盘1)
	int $0x13
	jc read_loader		// CF=0, 操作成功, 否则一直跳转

	// 跳转到C函数boot_entry
	jmp boot_entry

	// 原地跳转
	jmp .
	
	// 引导结束段
	.section boot_end, "ax"
boot_sig: .byte 0x55, 0xaa
