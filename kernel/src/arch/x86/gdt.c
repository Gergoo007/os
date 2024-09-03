#include <arch/x86/gdt.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <util/mem.h>

#include <gfx/console.h>

gdt_entry* gdt;

void gdt_init() {
	gdt = vmm_alloc();
	memset(gdt, 0, 0x1000);

	// Null entry
	memset(&gdt[0], 0, sizeof(gdt_entry));

	// Kernel kód
	gdt[1] = (gdt_entry) {
		.base1 = 0,
		.base2 = 0,
		.base3 = 0,

		.limit1 = 0xffff,
		.limit2 = 0xf,

		.access_byte.seg_desc = {
			.conforming = 0,
			.data_code = 1,
			.dpl = 0,
			.exec = 1,
			.present = 1,
			.rw = 1,
		},

		.limit_in_pages = 1,
		.long_mode = 1,
		.size = 0,
	};

	// Kernel adat
	gdt[2] = gdt[1];
	gdt[2].access_byte.seg_desc.exec = 0;

	// Felhasználó adat
	gdt[3] = gdt[2];
	gdt[3].access_byte.seg_desc.dpl = 3;

	// Felhasználó kód
	gdt[4] = gdt[1];
	gdt[4].access_byte.seg_desc.dpl = 3;

	gdt_load(&(gdtr) {
		sizeof(gdt_entry) * 7 - 1, // TSS nélkül
		(u64) gdt,
	});
}

void gdt_add_tss(tss* t) {
	MAKE_PHYSICAL(t);
	gdt[5].base1 = (u64)t;
	gdt[5].base2 = (u64)t >> 16;
	gdt[5].base3 = (u64)t >> 24;
	*(u32*)&gdt[6] = (u64)t >> 32;

	// _gdt->tss2.access_byte = 0x89;
	*(u8*)&(gdt[5].access_byte) = 0b10001001;
	gdt[5].access_byte.sys_seg_desc.dpl = 0;
	gdt[5].avl = 0;
	gdt[5].long_mode = 1;
	gdt[5].size = 0;
	gdt[5].limit_in_pages = 0;

	u32 limit = sizeof(tss) - 1; // Bitmap beletartozik?

	gdt[5].limit1 = limit & 0xf;
	gdt[5].limit2 = limit >> 4;
}
