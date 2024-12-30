#include <userspace/loader/loader.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <util/mem.h>

void (*elf_load(void* elf))() {
	Elf64_Ehdr* e = elf;
	Elf64_Phdr* p = elf + e->e_phoff;
	for (u32 i = 0; i < e->e_phnum; i++, p++) {
		if (p->p_type != PT_LOAD) continue;
		for (u64 a = p->p_vaddr; a < (p->p_vaddr + p->p_memsz); a += 0x1000) {
			map_page(a, (u64)pmm_alloc(), 0b11 | MAP_FLAGS_USER);
		}
		memcpy((void*)p->p_vaddr, elf + p->p_offset, p->p_filesz);
	}

	return (void*)e->e_entry;
}
