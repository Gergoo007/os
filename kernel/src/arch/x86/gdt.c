#include <arch/x86/gdt.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <util/mem.h>

#include <gfx/console.h>

gdt_entry* gdt;

void gdt_init() {
	gdt = vmm_alloc();
	memset(gdt, 0, 0x1000);

	// Null entry (0x00)
	memset(&gdt[0], 0, sizeof(gdt_entry));

	// Kernel kód (0x08)
	gdt[1] = (gdt_entry) {
		.base1 = 0,
		.base2 = 0,
		.base3 = 0,

		.limit1 = 0,
		.limit2 = 0,

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
		.avl = 0,
	};

	// Kernel adat (0x10)
	gdt[2] = gdt[1];
	gdt[2].access_byte.seg_desc.exec = 0;

	// Felhasználó adat (0x18)
	gdt[3] = gdt[2];
	// gdt[3].avl = 1;
	// gdt[3].limit1 = 0xffff;
	// gdt[3].limit2 = 0xf;
	gdt[3].access_byte.seg_desc.dpl = 3;

	// Felhasználó kód (0x20)
	gdt[4] = gdt[1];
	// gdt[4].avl = 1;
	// gdt[4].limit1 = 0xffff;
	// gdt[4].limit2 = 0xf;
	gdt[4].access_byte.seg_desc.dpl = 3;

	gdt_load(&(gdtr) {
		sizeof(gdt_entry) * 8 - 1,
		(u64) gdt,
	});
}

void gdt_add_tss(tss* t) {
	// // MAKE_PHYSICAL(t);
	// // t = (tss*)paging_lookup((u64)t);
	// printk("TSS @ %p\n", t);
	// gdt[5].base1 = (u64)t;
	// gdt[5].base2 = (u64)t >> 16;
	// gdt[5].base3 = (u64)t >> 24;
	// *(u32*)&gdt[6] = (u64)t >> 32;

	// // _gdt->tss2.access_byte = 0x89;
	// *(u8*)&(gdt[5].access_byte) = 0b10001001;
	// // *(u8*)&(gdt[5].access_byte) = 0x89;
	// gdt[5].access_byte.sys_seg_desc.dpl = 0;
	// gdt[5].avl = 0;
	// gdt[5].long_mode = 1;
	// gdt[5].size = 1;
	// gdt[5].limit_in_pages = 0;

	// u32 limit = sizeof(tss) - 1; // Bitmap beletartozik?

	// gdt[5].limit1 = limit & 0xf;
	// gdt[5].limit2 = limit >> 4;

	// printk("tss1 %p\n", *(u64*)&gdt[5]);
	// printk("tss2 %p\n", *(u64*)&gdt[6]);

	// MAKE_PHYSICAL(t);
	// t = (tss*)paging_lookup((u64)t);
	printk("TSS @ %p\n", t);
	gdt[5].base1 = (u64)t;
	gdt[5].base2 = (u64)t >> 16;
	gdt[5].base3 = (u64)t >> 24;
	*(u32*)&gdt[6] = (u64)t >> 32;

	// _gdt->tss2.access_byte = 0x89;
	*(u8*)&(gdt[5].access_byte) = 0b10001001;
	// *(u8*)&(gdt[5].access_byte) = 0x89;
	gdt[5].access_byte.sys_seg_desc.dpl = 0;
	gdt[5].avl = 0;
	gdt[5].long_mode = 1;
	gdt[5].size = 0;
	gdt[5].limit_in_pages = 0;

	u32 limit = sizeof(tss) - 1; // Bitmap beletartozik?

	gdt[5].limit1 = limit & 0xf;
	gdt[5].limit2 = limit >> 4;

	printk("tss1 %p\n", *(u64*)&gdt[5]);
	printk("tss2 %p\n", *(u64*)&gdt[6]);
}
