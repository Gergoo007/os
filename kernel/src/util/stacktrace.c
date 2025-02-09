#include <util/stacktrace.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <util/string.h>

static Elf64_Sym* get_function(u64 address) {
	Elf64_Sym* nearest = ksymtab;
	for (Elf64_Sym* s = ksymtab; (void*)s < ((void*)ksymtab+ksymtab_size); s++) {
		if (s->st_value > address) continue;

		if ((address - s->st_value) < (address - nearest->st_value))
			nearest = s;
	}

	return nearest;
}

static char* get_name(Elf64_Sym* s) {
	if (s->st_name < kstrtab_size) {
		return kstrtab + s->st_name;
	}
	return 0;
}

void stacktrace(u64 currentaddr, stackframe* rbp) {
#ifdef STACKTRACE
	#define print(fmt, ...) printk(fmt, __VA_ARGS__); \
							sprintk(fmt, __VA_ARGS__)

	if (con_cx != 0)
		cputc('\n');

	Elf64_Sym* s;
	if (currentaddr) {
		s = get_function(currentaddr);
		print("@ [%p] %s+0x%x\n", rbp->rip, get_name(s), currentaddr - s->st_value);
	}

	extern void kmain(void* boot_info, u64 preloader_img_len);

	do {
		s = get_function(rbp->rip);
		print("  [%p] %s+0x%x\n", rbp->rip, get_name(s), rbp->rip - s->st_value);

		rbp = rbp->rbp;
		if (!((u64)rbp & 0xffff800000000000)) {
			error("Bad rbp: %p", rbp);
			while (1);
		}
	} while (get_function(rbp->rip)->st_value != (u64)kmain);
	s = get_function(rbp->rip);
	print("  [%p] %s+0x%x\n", rbp->rip, get_name(s), rbp->rip - s->st_value);

	#undef print
#elifdef STACKTRACE_MINIMAL
	// #define print(fmt, ...) printk(fmt, __VA_ARGS__); \
	// 						sprintk(fmt, __VA_ARGS__)

	// Elf64_Sym* s;
	// if (currentaddr) {
	// 	s = get_function(currentaddr);
	// 	print("@ %p\n", s->st_value);
	// }

	// extern void kmain(void* boot_info, u64 preloader_img_len);

	// do {
	// 	s = get_function(rbp->rip);
	// 	print("  %p\n", s->st_value);

	// 	rbp = rbp->rbp;
	// } while (get_function(rbp->rip)->st_value != (u64)kmain);
	// s = get_function(rbp->rip);
	// print("  %p\n", s->st_value);

	// #undef print
#endif
}
