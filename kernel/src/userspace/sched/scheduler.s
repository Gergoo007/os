.global sched_tick

// #define MAX_PROCS 32768

.extern sched_counter # u32 sched_counter;
.extern proctable # process* proctable

// Lejárt a SCHED_TICK; jöhet a következő folyamat
// void sched_tick();
sched_tick:
	xor %rcx, %rcx
	mov sched_counter(%rip), %ecx
	shl $5, %rcx # 256 byte egy struct de 8 a max az addressnél ezért az indexet kell szorozni
	mov proctable, %rbx
	lea (%rbx, %rcx, 8), %rdi
	add $16, %rdi
	push %rdi

	cli
	call lapic_eoi

	pop %rdi
	mov %rdi, %rsp
	mov -0x8(%rdi), %rax
	mov %rax, %cr3

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8

	pop %rax
	movq %rax, %cr2

	pop %rbp
	pop %rsi
	pop %rdi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	add $0x10, %rsp
	popfq
	sti
	iretq
