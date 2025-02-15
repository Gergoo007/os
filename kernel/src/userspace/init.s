.global userexec
.global userinit
.global kstack
.global ustack

.extern cputs

.section .bss
kstack:
	.quad 0
ustack:
	.quad 0

.section .text
# %rcx: return cím
# %r11: %rflags
on_syscall:
	movq %rsp, ustack(%rip)
	movq kstack(%rip), %rsp

	cmp $0, %eax
	je .exit

	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rdi
	push %rsi
	push %rbp
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	push $.returnto

.puts:
	jmp cputs

.returnto:
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rbp
	pop %rsi
	pop %rdi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	movq ustack(%rip), %rsp
	sysretq

.exit:
	jmp khang

# %rdi: entry
# %rsi: uj stack
userexec:
	# STAR MSR
	movq $0xC0000081, %rcx
	# STAR 63:48 + 16: user CS
	# STAR 63:48 + 08: user SS
	mov $0x00000000, %eax
	mov $0x00130008, %edx
	wrmsr

	# LSTAR MSR
	movq $0xC0000082, %rcx
	# LSTAR: syscall %rip
	movabs $on_syscall, %rdx
	shr $32, %rdx
	movabs $on_syscall, %rax
	wrmsr

	# Többi szegmens beállítása 0x18-ra (offsetof gdt->udata)
	mov $0x1b, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	swapgs

	movq %rsp, kstack(%rip)

	push %rbp
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rdi
	push %rsi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	# Felhasználói stack
	mov %rsi, %rsp

	# %rip
	mov %rdi, %rcx
	# %eflags
	mov $0x202, %r11

	sysretq

userinit:
	# STAR MSR
	movq $0xC0000081, %rcx
	# STAR 63:48 + 16: user CS
	# STAR 63:48 + 08: user SS
	mov $0x00000000, %eax
	mov $0x00130008, %edx
	wrmsr

	# LSTAR MSR
	movq $0xC0000082, %rcx
	# LSTAR: syscall %rip
	movabs $on_syscall, %rdx
	shr $32, %rdx
	movabs $on_syscall, %rax
	wrmsr

	ret
