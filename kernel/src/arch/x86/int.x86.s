.altmacro

.macro GLOBISR f, t
.global exc\f
.if \t-\f
GLOBISR %f+1,\t
.endif
.endm

.macro DECLRISR_NOFLAG f, t
exc\f:
	pushfq
	cli
	push $\f
	jmp pushall
.if \t-\f
DECLRISR_NOFLAG %f+1,\t
.endif
.endm

.macro DECLRISR f, t
exc\f:
	pushfq
	cli
	push $0
	push $\f
	jmp pushall
.if \t-\f
DECLRISR %f+1,\t
.endif
.endm

GLOBISR 0, 100

.section .data
.align 16
sse_state:
	.skip 0x512

.section .text
# Exceptionök
DECLRISR 0, 9
DECLRISR_NOFLAG 10, 14
DECLRISR 15, 19

# Megszakítások
DECLRISR 64, 72

.extern x86_introutine

pushall:
	fxsave sse_state
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
	fxrstor sse_state

	add $0x10, %rsp # hibakód + vektor

	popfq

	iretq
