.global ap_starter
.global apcr3
.extern gdtr
.extern kmain

.org 0x8000
.code16
ap_starter:
	cli
	cld
	ljmp $0, $0x8007
nullcs:
	movl 0x8500, %eax
	mov %eax, %cr3

	mov $0xc0000080, %ecx
	rdmsr

	or $0x00000100, %eax
	wrmsr

	# paging
	mov %cr4, %eax
	or $0x30, %eax # PAE
	mov %eax, %cr4

	mov $0x11, %eax
	mov %eax, %cr0

	mov $0x80000011, %eax
	mov %eax, %cr0

	lgdt 0x8110

	ljmp $8,$0x8200

.org 0x8100
gdt:
	.quad 0
	.quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
gdt_end:
gdtr:
	.word 0xf
	.quad 0x8100

.org 0x8200
.code64
.extern onsmp
.extern cpus_up
longmode:
	# TODO: Az APIC ID nem biztos hogy sorrendbe van 0-tól
	mov $1, %eax
	cpuid

	shr $24, %ebx
	xor %rcx, %rcx
	movzwl %bx, %ecx
	dec %ecx

	mov smp_stacks, %rbx

	mov (%rbx, %rcx, 8), %rsp
	mov (%rbx, %rcx, 8), %rbp

	lea onsmp, %rax
	jmp *%rax
