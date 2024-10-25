/**
 *
 * 二级引导, 负责进行硬件检测, 进入保护模式, 然后加载内核, 并跳转至内核运行
 *
 */
  	# 16位代码, 务必加上
  	.code16
 	.text
 	.extern loader_entry
	.global _start

# 0x8000
_start:

	# 栈和段等沿用之前的设置
	# 不直接跳到loader_entry的原因: boot不知道loader_entry在哪儿
	# 所以需要用汇编, 以便_start处于整个bin文件(后面会改成elf文件)的开头, 然后再跳转

	# 跳转到引导器的入口函数
	jmp loader_entry


////////////////////////////////////////////////////////////

	# 接下来是32位代码
	.code32
	.text
	.global protect_mode_entry
	.global loader_kernel

/**
 * 保护模式入口点，在enter_protect_mode中被调用
 */
protect_mode_entry:
	# 正式进入保护模式

	# 设置段选择子
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# 跳转至C函数	
	jmp $8, $loader_kernel