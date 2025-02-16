.global userinit
.global krsp
.global krbp
.global ursp
.global urbp

.extern cputs

.extern lapic_timer
.extern sched_remove_process

.section .bss
// krsp:
// 	.quad 0
// krbp:
// 	.quad 0
syscallstack_start:
	.skip 0x8000
syscallstack_end:
ursp:
	.quad 0
urbp:
	.quad 0

.section .text
# %rcx: return cím
# %r11: %rflags
on_syscall:
	# LAPIC időzítő kikapcsolása, ezáltal a context switchek megállítása
	// movl $0x10044, lapic_timer(%rip)

	movq %rsp, ursp(%rip)
	movq %rbp, urbp(%rip)

	// movq krsp(%rip), %rsp
	// movq krbp(%rip), %rbp
	movabs $syscallstack_end, %rsp
	movabs $syscallstack_end, %rbp

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

	movq ursp(%rip), %rsp
	movq urbp(%rip), %rbp

	# LAPIC timer visszakapcsolása
	// movl $0x00044, lapic_timer(%rip)
	sysretq

.exit:
	# Folyamat törlése
	mov sched_counter(%rip), %rdi
	call sched_remove_process
	sti # ez nem tom mért kell de kell real hw-en

	# várakozás a context switchre
	# TODO: cs. erőltetése
	jmp .

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
