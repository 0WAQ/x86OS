/**
 * 二级引导器, 负责进行硬件检测, 进入保护模式, 加载并跳转至内核
 */

# 16位实模式
.code16
.text

.extern loader16_entry
.global _start

# 0x8000
_start:
	jmp loader16_entry


# ############################################################


# 32位代码
.code32
.text

.global loader32_protect_mode_entry
.extern loader32_enter_kernel

# 保护模式入口点, 在 loader32_enter_protect_mode 中被调用
loader32_protect_mode_entry:
	# 设置段选择子
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# 跳转至C函数	
	jmp $8, $loader32_enter_kernel
