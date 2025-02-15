.global gdt_load

.global gdtr

.text
gdt_load:
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
