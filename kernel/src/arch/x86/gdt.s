.global gdt_load

.global gdtr

.bss
gdtr:
	.quad 0
apidtr:
	.quad 0

.text
gdt_load:
	mov %rdi, gdtr
	lgdt (%rdi)

	# Far jump az új kód szegmensbe
	push $0x08 # kernel kód
	lea .others, %rax
	push %rax
	retfq

.others:
	# Többi szegmens regiszter beállítása
	mov $0x10, %ax # kernel adat
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	ret
