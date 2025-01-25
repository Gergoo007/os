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

void stacktrace(u64 currentaddr, stackframe* rbp) {
	#define print(fmt, ...) printk(fmt, __VA_ARGS__); \
							sprintk(fmt, __VA_ARGS__)

	Elf64_Sym* s;
	if (currentaddr) {
		s = get_function(currentaddr);
		print("@ %s+0x%x\n", kstrtab + s->st_name, currentaddr - s->st_value);
	}

	extern void kmain(void* boot_info, u64 preloader_img_len);

	do {
		s = get_function(rbp->rip);
		print("  %s+0x%x\n", kstrtab + s->st_name, rbp->rip - s->st_value);

		rbp = rbp->rbp;
	} while (get_function(rbp->rip)->st_value != (u64)kmain);
	s = get_function(rbp->rip);
	print("  %s+0x%x\n", kstrtab + s->st_name, rbp->rip - s->st_value);

	#undef print
}
