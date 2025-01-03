.global pmain

.extern _binary_kernel_start
.extern _binary_kernel_end
.extern IMAGE_START
.extern IMAGE_END

.extern _binary_out_ksymtab_start
.extern _binary_out_kstrtab_start
.extern _binary_out_kshstrtab_start

.code32
.section .text
.align 8
mb2_hdr:
	// magic
	.long 0xe85250d6
	// ISA: i386
	.long 0
	// Header length.
	.long mb2_hdr_end - mb2_hdr
	// checksum
	.long -(0xe85250d6 + 0 + (mb2_hdr_end - mb2_hdr))

	.align 8
	fb_tag:
		.short 5
		.short 0
		.long 20
		.long 1024
		.long 768
		.long 32
	fb_tag_end:

	.align 8
	terminator:
		.short 0
		.short 0
		.long 8
	terminator_end:
mb2_hdr_end:

.section .bss
.align 8
stack:
	.skip 0x10000
stack_end:
.align 0x1000
pml4:
	.skip 0x1000
pdp_pl:
	.skip 0x1000
pd_pl1:
	.skip 0x1000
pd_pl2:
	.skip 0x1000
pd_pl3:
	.skip 0x1000
pd_pl4:
	.skip 0x1000
pdp_k:
	.skip 0x1000
pd_k:
	.skip 0x1000
boot_info:
	.skip 0x3000
mb_fb:
	.skip 4

.section .data
.align 0x1000
gdt:
	.quad 0
	.quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
gdt_end:
gdtr:
	.word gdt_end - gdt - 1
	.quad gdt
fcw:
	.int 0x037F

.equ CACHE_WB, 3
.equ CACHE_WC, 1
.equ CACHE_WT, 2
.equ CACHE_UC, 0

.equ MAP_DEF, 0b11011 # Present, RW, WB
.equ MAP_FB,  0b01011 # Present, RW, WC

// Uncached gyorsabb mint a write-back..?
#.equ MAP_DEF_2M, 0b10011011 # Present, RW, WB, 2MiB
 .equ MAP_DEF_2M, 0b10000011 # Present, RW, UC, 2MiB
#.equ MAP_DEF_2M, 0b10010011 # Present, RW, WT, 2MiB
 .equ MAP_FB_2M,  0b10001011 # Present, RW, WC, 2MiB

// .equ MAP_DEF, 0b00111 # Present, RW, WB
// .equ MAP_FB,  0b00111 # Present, RW, WC

// // Uncached gyorsabb mint a write-back..?
// #.equ MAP_DEF_2M, 0b10011011 # Present, RW, WB, 2MiB
//  .equ MAP_DEF_2M, 0b10000111 # Present, RW, UC, 2MiB
// #.equ MAP_DEF_2M, 0b10010011 # Present, RW, WT, 2MiB
//  .equ MAP_FB_2M,  0b10000111 # Present, RW, WC, 2MiB

.section .text
pmain:
	cli

	mov $stack_end, %esp
	mov %esp, %ebp

	# PAT MSR kitöltése
	mov $0x277, %ecx
	rdmsr
	mov $0x00010406, %edx # Magasabb 32
	mov $0x00010406, %eax # Alacsonyabb 32
	wrmsr

	mov %ebx, %edi
	push %edi

	mov %edi, %esi
	mov $boot_info, %edi
	mov (%esi), %ecx
	# 4 byte-onként mozgatom az adatot
	shr $2, %ecx
	inc %ecx

	rep movsl

	mov $boot_info, %edi
	mov %edi, (%esp)

	# symtab, strtab, shstrtab
	mov $boot_info, %edi
	addl (%edi), %edi

	movl $_binary_out_ksymtab_start, 0(%edi)
	movl $_binary_out_ksymtab_size, 4(%edi)
	movl $_binary_out_kstrtab_start, 8(%edi)
	movl $_binary_out_kstrtab_size, 12(%edi)
	movl $_binary_out_kshstrtab_start, 16(%edi)
	movl $_binary_out_kshstrtab_size, 20(%edi)

	mov (%esp), %edi

	# TODO: Előre kitöltött táblázatok
	mov $pdp_pl, %eax
	or $MAP_DEF, %eax
	mov %eax, pml4

	mov $pdp_pl, %ebx

	mov $pd_pl1, %eax
	or $MAP_DEF, %eax
	mov %eax, (%ebx)

	mov $pd_pl2, %eax
	or $MAP_DEF, %eax
	mov %eax, 8(%ebx)

	mov $pd_pl3, %eax
	or $MAP_DEF, %eax
	mov %eax, 16(%ebx)

	mov $pd_pl4, %eax
	or $MAP_DEF, %eax
	mov %eax, 24(%ebx)

	mov $MAP_DEF_2M, %eax
	mov $pd_pl1, %ebx
	xor %ecx, %ecx
kitolt_pd_pl1:
	mov %eax, (%ebx, %ecx, 8)

	add $0x200000, %eax

	inc %ecx
	cmp $512, %ecx
	jne kitolt_pd_pl1

	mov $pd_pl2, %ebx
	xor %ecx, %ecx
kitolt_pd_pl2:
	mov %eax, (%ebx, %ecx, 8)

	add $0x200000, %eax

	inc %ecx
	cmp $512, %ecx
	jne kitolt_pd_pl2

	mov $pd_pl3, %ebx
	xor %ecx, %ecx
kitolt_pd_pl3:
	mov %eax, (%ebx, %ecx, 8)

	add $0x200000, %eax

	inc %ecx
	cmp $512, %ecx
	jne kitolt_pd_pl3

	mov $pd_pl4, %ebx
	xor %ecx, %ecx
kitolt_pd_pl4:
	mov %eax, (%ebx, %ecx, 8)

	add $0x200000, %eax

	inc %ecx
	cmp $512, %ecx
	jne kitolt_pd_pl4

	mov $pdp_k, %eax
	or $MAP_DEF, %eax
	mov $pml4, %ebx
	add $(511 * 8), %ebx
	mov %eax, (%ebx)

	mov $pd_k, %eax
	or $MAP_DEF, %eax
	mov $pdp_k, %ebx
	add $(511 * 8), %ebx
	mov %eax, (%ebx)

	mov $pdp_pl, %eax
	or $MAP_DEF, %eax
	mov $pml4, %ebx
	add $(256 * 8), %ebx
	mov %eax, (%ebx)

	# Szabad memória keresése a kernelnek
	add $8, %edi

tag_vizsgal:
	# type == memory map
	cmpl $6, (%edi)
	je mmap_megvan

	cmpl $8, (%edi)
	je framebuffer_megvan

tobbitag:
	mov 4(%edi), %eax
	add %eax, %edi

	# Kell alignolni 8-ra?
	push %edi
	andl $7, %edi
	pop %edi
	jz noalign

	or $7, %edi
	inc %edi

noalign:
	cmpl $0, (%edi)
	jne tag_vizsgal
	
	cmpl $8, 4(%edi)
	jne tag_vizsgal

mb2_fejlec_map:
	push %ebx
	mov $0x00ff0000, %ebx
	// call draw_rect
	pop %ebx

	# Multiboot2 header -> 0xffffffffca000000
	# pdi: 80
	mov (%esp), %edi
	and $0xffe00000, %edi # 2 MiB align lefele
	or $MAP_DEF_2M, %edi
	mov $pd_k, %eax
	mov %edi, 640(%eax)

	mov $pml4, %eax
	mov %eax, %cr3

	# CR4.PAE = 1
	mov %cr4, %eax
	or $(1 << 5), %eax
	mov %eax, %cr4

	# EFER.LME = EFER.SYSCALL = 1
	movl $0xc0000080, %ecx
	rdmsr
	orl $0x101, %eax
	wrmsr

	# CR0.PG = 1
	mov %cr0, %eax
	or $(1 << 31), %eax
	mov %eax, %cr0

	lgdt gdtr

	push %ebx
	mov $0x0000ff00, %ebx
	// call draw_rect
	pop %ebx

	jmp $0x08,$messze

framebuffer_megvan:
	mov 8(%edi), %eax
	mov 12(%edi), %ebx

	mov %edi, mb_fb

	mov $pd_k, %ecx
	or $MAP_FB_2M, %eax
	mov %eax, 128(%ecx)
	mov %ebx, 132(%ecx)

	add $0x200000, %eax
	mov %eax, 136(%ecx)
	mov %ebx, 140(%ecx)

	add $0x200000, %eax
	mov %eax, 144(%ecx)
	mov %ebx, 148(%ecx)

	add $0x200000, %eax
	mov %eax, 152(%ecx)
	mov %ebx, 156(%ecx)

	jmp tobbitag

mmap_megvan:
	push %edi
	add $8, %edi
	mov (%edi), %ecx # entry méret

	add $8, %edi

	# %edi: &(entries[0])
szabad_e:
	cmpl $1, 16(%edi)
	je tovabb

vissza:
	add %ecx, %edi
	jmp szabad_e

# TODO: (%edi)-ban van a szabad mem címe, de nem fix
# hogy 2M aligned, ebben az esetben crash
# 4K paging kell ide, page table mehet akár ebbe a régióba
tovabb:
	// mov (%edi), %eax
	mov $0, %eax
	mov 4(%edi), %ecx
	or $MAP_DEF_2M, %eax
	mov $pd_k, %ebx
	add $(112 * 8), %ebx
	mov %eax, (%ebx)
	mov %ecx, 4(%ebx)

	pop %edi

	jmp tobbitag

# %ebx: szín
draw_rect:
	push %eax
	push %ecx
	push %edi
	mov mb_fb, %edi
	mov 8(%edi), %eax # base
	mov 16(%edi), %edi # pitch (width*bpp)

	# %edx: y
	# %ecx: x
	xor %edx, %edx
.col:
		xor %ecx, %ecx
	.row:
		mov %ebx, (%eax, %ecx, 4)
		inc %ecx
		cmp $51, %ecx
		jne .row
	inc %edx
	add %edi, %eax
	cmp $51, %edx
	jne .col

	pop %edi
	pop %ecx
	pop %eax
	ret

.code64
# %ebx: szín
draw_rect64:
	push %rax
	push %rcx
	push %rdi
	mov mb_fb, %edi
	mov 8(%edi), %eax # base
	mov 16(%edi), %edi # pitch (width*bpp)

	# %edx: y
	# %ecx: x
	xor %edx, %edx
.col64:
		xor %ecx, %ecx
	.row64:
		mov %ebx, (%eax, %ecx, 4)
		inc %ecx
		cmp $51, %ecx
		jne .row64
	inc %edx
	add %edi, %eax
	cmp $51, %edx
	jne .col64

	pop %rdi
	pop %rcx
	pop %rax
	ret

messze:
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs
	mov %ax, %fs
	mov %ax, %ss

	push %rbx
	mov $0x000000ff, %ebx
	// call draw_rect64
	pop %rbx

	# FPU init
	fninit
	fldcw fcw

	# SSE engedélyezése
	mov %cr0, %rax
	and $(~(1 << 2)), %rax	# CR0.EM (x87)
	or $(1 << 1), %rax		# CR0.MP (monitor co-proc)
	or $(1 << 5), %rax		# CR0.NE (num error)
	mov %rax, %cr0

	# Egyéb hülyeségek (FXSAVE, FXSTOR, unmasked cuccos)
	mov %cr4, %rax
	or $(1 << 9), %rax
	or $(1 << 10), %rax
	mov %rax, %cr4

	# Van SSE2?
	mov $1, %eax
	xor %ecx, %ecx
	cpuid
	and $(1 << 26), %edx

	movabs $_binary_kernel_start, %rax
	cmpl $0x464c457f, (%rax) # ELF
	jne hiba

	xor %ebx, %ebx
	mov 32(%rax), %ebx # Elf64_Ehdr->e_phoff
	lea (%rax, %rbx, 1), %rcx

	xor %rbx, %rbx
	mov 54(%rax), %bx # Elf64_Ehdr->e_phentsize

	xor %rsi, %rsi
	mov 56(%rax), %si # Elf64_Ehdr->e_phnum

	# %rcx: első phdr
	# %rbx: entry size

	# PT_LOAD: 1
szegmens:
	cmp $0, %rsi
	je vege

	# Ha nem PT_LOAD akkor nem kell vele foglalkozni
	cmpl $1, (%rcx)
	jne kovetkezo

	# Szegmens betöltése
	# %rdx: innen
	# %rdi: ide
	mov 8(%rcx), %rdx
	add %rax, %rdx
	xor %rdi, %rdi
	mov 16(%rcx), %rdi
	mov 32(%rcx), %r8

	mov 40(%rcx), %r10 # memsz
	call memset

	call memcpy

	# Következő PHDR
kovetkezo:
	add %rbx, %rcx
	dec %rsi
	jmp szegmens

vege:
	# TODO: BSS nulla
	mov $IMAGE_END, %rsi
	sub $IMAGE_START, %rsi
	add $0x1000, %rsi

	push %rbx
	mov $0x00ffffff, %ebx
	// call draw_rect64
	pop %rbx

	mov 24(%rax), %rbx # e_entry
	jmp *%rbx

hiba:
	push %rbx
	mov $0x00ff0000, %ebx
	// call draw_rect64
	pop %rbx

	mov $0xdeadbeef, %rax
	jmp .

memcpy:
	push %rcx
	push %rsi
	mov %r8, %rcx
	mov %rdx, %rsi
	rep movsb
	pop %rsi
	pop %rcx
	ret

memset:
	push %rdi
	push %rax
	push %rcx

	mov %r10, %rcx
	mov $0, %al
	add %r8, %rdi
	rep stosb

	pop %rcx
	pop %rax
	pop %rdi

	ret

log:
	push %rax
	push %rdx

	mov $'c', %al
	mov $0x3f8, %dx
	outb %al, %dx

	pop %rdx
	pop %rax
