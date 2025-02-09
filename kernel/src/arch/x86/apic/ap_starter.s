.global ap_starter
.global apcr3
.extern gdtr
.extern kmain

.org 0x8000
.code16
ap_starter:
	cli
	cld
	ljmp    $0, $0x8040
	.align 16
_L8010_GDT_table:
	.long 0, 0
	.long 0x0000FFFF, 0x00CF9A00    # flat code
	.long 0x0000FFFF, 0x008F9200    # flat data
	.long 0x00000068, 0x00CF8900    # tss
_L8030_GDT_value:
	.word _L8030_GDT_value - _L8010_GDT_table - 1
	.long 0x8010
	.long 0, 0
	.align 64
_L8040:
	xorw    %ax, %ax
	movw    %ax, %ds
	lgdtl   0x8030
	movl    %cr0, %eax
	orl     $1, %eax
	movl    %eax, %cr0
	ljmp    $8, $0x8060

.align 32
.code32
_L8060:
	movw    $16, %ax
	movw    %ax, %ds
	movw    %ax, %ss
	# get our Local APIC ID
	mov     $1, %eax
	cpuid
	shrl    $24, %ebx
	movl    %ebx, %edi

	# cr3
	// mov 0x80ac, %eax
	mov $0x15d000, %eax
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

	lgdt 0x9010

	ljmp $8,$0x901c

.org 0x9000
gdt:
	.quad 0
	.quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
gdt_end:
gdtr:
	// .word gdt_end - gdt - 1
	// .quad gdt
	.word 0xf
	.quad 0x9000

.org 0x901c
.code64
longmode:
	jmp .

apcr3:
	.quad
