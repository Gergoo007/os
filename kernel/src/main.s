.global kentry

.extern kmain

.code64
.section .text
kentry:
	// %edi lett pusholva, ami csak 4
	pop %rdi
	sub $4, %rsp
	movabsq $0x1fffff, %rax
	and %rax, %rdi

	add $0xffffffffca000000, %rdi

	jmp kmain
