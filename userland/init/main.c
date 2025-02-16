void _start() {
	asm volatile ("movq $1, %%rax\nmovq %0, %%rdi\nsyscall" :: "r"("helo from inti\n") : "rax", "rdi");
	asm volatile ("xor %%rax, %%rax\nsyscall" ::: "rax");
	while (1);
}
