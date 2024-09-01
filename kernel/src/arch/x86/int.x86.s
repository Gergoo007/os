.global exc0
.global exc1
.global exc2
.global exc3
.global exc4
.global exc5
.global exc6
.global exc7
.global exc8
.global exc9
.global exc10
.global exc11
.global exc12
.global exc13
.global exc14
.global exc15
.global exc16
.global exc17
.global exc18
.global exc19

.global exc64

.extern x86_introutine

pushall:
	// add $0x10, %rsp
	// jmp .
	// iretq

	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rdi
	push %rsi
	push %rbp

	movq %cr2, %rax
	push %rax
	
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	movq %rsp, %rdi
	movabsq $x86_introutine, %rax
	cld
	call *%rax

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rax # cr2
	pop %rbp
	pop %rsi
	pop %rdi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
	pop %r15
	
	add $0x8, %rsp
	// jmp .

	iretq

exc0:
	cli
	push $0
	push $0
	jmp pushall
exc1:
	cli
	push $0
	push $1
	jmp pushall
exc2:
	cli
	push $0
	push $2
	jmp pushall
exc3:
	cli
	push $0
	push $3
	jmp pushall
exc4:
	cli
	push $0
	push $4
	jmp pushall
exc5:
	cli
	push $0
	push $5
	jmp pushall
exc6:
	cli
	push $0
	push $6
	jmp pushall
exc7:
	cli
	push $0
	push $7
	jmp pushall
exc8:
	cli
	push $8
	jmp pushall
exc9:
	cli
	push $0
	push $10
	jmp pushall
exc10:
	cli
	push $10
	jmp pushall
exc11:
	cli
	push $11
	jmp pushall
exc12:
	cli
	push $12
	jmp pushall
exc13:
	cli
	push $13
	jmp pushall
exc14:
	cli
	push $14
	jmp pushall
exc15:
	cli
	push $0
	push $14
	jmp pushall
exc16:
	cli
	push $0
	push $16
	jmp pushall
exc17:
	cli
	push $17
	jmp pushall
exc18:
	cli
	push $0
	push $18
	jmp pushall
exc19:
	cli
	push $0
	push $19
	jmp pushall

exc64:
	cli
	push $0
	push $64
	jmp pushall
