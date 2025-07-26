/**
 * boot:
 * 	启动时由硬件加载运行, 用于加载并跳转至二级引导器(0x8000处), 
 * 	应在磁盘的第一个扇区中, 内存首地址在 0x7c00 处, 由 BIOS 加载并跳转
 */

# 16位代码段, 实模式
.code16
.text

.global _start

# 0x7c00
_start:
	mov $0, %eax	# 此处会发生异常是因为在gdb中要求它运行到0x7c00处
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# 设置栈顶在0x7c00
	# 参考链接: https://wiki.osdev.org/Memory_Map_(x86)
	mov $_start, %esp

	mov $boot_msg, %si
	calll show_boot_msg

# 从磁盘中加载 loader, 只支持 disk1
# 参考链接: https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)
read_loader:
	mov $0x8000, %bx	# 将loader读取到0x8000处
	mov $0x02, %cx		# ch: 磁道号, cl起始扇区号
	mov $0x02, %ah		# ah: 0x02读磁盘命令
	mov $64,   %al		# al: 读取的扇区数量, 必须小于128, 暂设置成32KB
	mov $0x0080, %dx	# dh: 磁头号, dl: 驱动器号0x80(磁盘1)
	int $0x13
	jc read_loader		# CF=0, 操作成功, 否则一直跳转

# 跳转到 loader的入口点
jmp_loader:
	mov $0x8000, %eax
	jmp *%eax

show_boot_msg:
	push %ax

  show_boot_msg_loop:
	mov $0xe, %ah
	mov (%si), %al

	test %al, %al
	jz show_boot_msg_end

	int $0x10

	inc %si
	jmp show_boot_msg_loop

  show_boot_msg_end:
	pop %ax
	retl

boot_msg:
	.string "...Booting...\r\n\0"

# boot 必须以 0xaa55 结尾
.section boot_end, "ax"		# boot_end 被设置为: 0x7dfe
	.byte 0x55, 0xaa
