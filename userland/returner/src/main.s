.global _start

.section .data
text:
	.asciz "userspace says ok\n"

.section .text
_start:
	# Kiíratás (printk) syscall
	mov $1, %eax
	movabs $text, %rdi
	syscall

	# Exit syscall
	mov $0, %eax
	syscall
