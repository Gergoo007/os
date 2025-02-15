__attribute__((noreturn)) void exit() {
	asm volatile ("syscall" :: "eax"(0));
	while (1);
}

void puts(char* s) {
	asm volatile ("syscall" :: "eax"(1), "rdi"(s));
}

void _start() {
	puts("hello world from exe\n");
	exit();
}
