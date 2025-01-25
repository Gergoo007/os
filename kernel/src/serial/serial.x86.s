.global outb
.global outw
.global outl

.global inb
.global inw
.global inl

.global sputc
.global sputs

# void outb(u8 b, u16 port);
outb:
	mov %si, %dx
	mov %dil, %al
	outb %al, %dx
	ret

# void outw(u16 b, u16 port);
outw:
	mov %si, %dx
	mov %di, %ax
	outw %ax, %dx
	ret

# void outl(u32 b, u16 port);
outl:
	mov %si, %dx
	mov %edi, %eax
	outl %eax, %dx
	ret

# u8 inb(u16 port);
inb:
	mov %di, %dx
	inb %dx, %al
	ret

# u16 inw(u16 port);
inw:
	mov %di, %dx
	inw %dx, %ax
	ret

# u32 inl(u16 port);
inl:
	mov %di, %dx
	inl %dx, %eax
	ret

// # void sputc(const char c);
// sputc:
// 	mov %dil, %al
// 	mov $0x3f8, %dx
// 	outb %al, %dx
// 	ret

# u8 sputc(const char* c);
sputc:
	mov $0x3fd, %dx

	# El lett küldve az előző karakter?
.wait:
	inb %dx, %al
	and $0x20, %al
	cmp $0, %al
	je .wait

	mov $0x3f8, %dx
	mov (%rdi), %al
	
	cmp $'\n', %al
	jne .notlf

	mov $'\n', %al
	outb %al, %dx

	# El lett küldve az előző karakter?
	mov $0x3fd, %dx
.wait3:
	inb %dx, %al
	and $0x20, %al
	cmp $0, %al
	je .wait3

	mov $0x3f8, %dx
	mov $'\r', %al
	outb %al, %dx
	ret

.notlf:
	outb %al, %dx
	mov $1, %al
	mov $1, %ah
	ret

# void sputs(const char* str);
sputs:
	cmpb $0, (%rdi)
	je .ret

	# El lett küldve az előző karakter?
	mov $0x3fd, %dx
.wait2:
	inb %dx, %al
	and $0x20, %al
	cmp $0, %al
	je .wait2

	# El, mehet a következő
	mov $0x3f8, %dx
	movb (%rdi), %al
	outb %al, %dx

	inc %rdi
	jmp sputs
.ret:
	ret
