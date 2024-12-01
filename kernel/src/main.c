#include <gfx/console.h>
#include <util/mem.h>
#include <util/string.h>
#include <serial/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>
#include <arch/x86/pic.h>
#include <arch/x86/clocks/pit.h>
#include <arch/x86/clocks/tsc.h>
#include <dtree/tree.h>
#include <ps2/kbd.h>
#include <acpi/acpi.h>
#include <boot/multiboot2.h>
#include <userspace/init.h>
#include <fs/vfs/vfs.h>
#include <fs/gpt.h>
#include <fs/fat32/fat32.h>
#include <fs/vfs/ramfs.h>

extern _attr_noret void khang();
extern u16* unilookup;

// TODO: kfree megjavítása

// TODO: 4K szektorméret AHCI-ba
// TODO: drivers & driverif implementáció, drivers mappa, modulok
// TODO: Külön fordítani LAI-t
// TODO: AVX memset
// TODO: Serial -> arch/x86
// TODO: PS/2: Van-e eszköz az első porton egyáltalán?

_attr_align_stack _attr_noret void kmain(void* boot_info, u64 preloader_img_len) {
	// Rendes hardwaren ez befagyasztja a rendszert...
	// serial_init(0x3f8);

	// asm volatile ("outw %0, %1" :: "a"((u16)0x8A00), "d"((u16)0x8A00));

	asm volatile ("mov %%cr3, %0" : "=r"(pml4));

	u64 cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= (1 << 4);
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));

	multiboot2_parse(boot_info, preloader_img_len);

	// TODO: ha igazi hw-en nem jó, ezt nem szabad futtatni
	pic_init();

	// Framebuffer előkészítése
	fb_main.base = FB_VADDR;
	fb_main.size = 1024*768*4;
	fb_main.width = 1024;
	fb_main.height = 768;
	con_init();

	printk("Heap %p\n", heap_base_phys);

	printk("Osszes memoria: %dM\n", pmm_mem_all >> 20);
	printk("Szabad memoria: %dM\n", pmm_mem_free >> 20);

	paging_init();
	MAKE_VIRTUAL(boot_info);
	MAKE_VIRTUAL(pml4);
	pml4->entries[0].addr = 0;

	u64 rsp, rbp;
	asm volatile ("movq %%rsp, %0\nmovq %%rbp, %1" : "=r"(rsp), "=r"(rbp));
	rsp |= 0xffff800000000000;
	rbp |= 0xffff800000000000;
	asm volatile ("movq %0, %%rsp\nmovq %1, %%rbp" :: "r"(rsp), "r"(rbp));

	unilookup = (u16*)((u64)unilookup | 0xffff800000000000);

	vmm_init();

	gdt_init();
	idt_init();

	vfs_init();

	// vfs_dir* d = kmalloc(sizeof(vfs_dir) + 8); // Egy child lesz

	// vfs_file* f = kmalloc(sizeof(vfs_file));
	// f->name = "tesztfile";
	// f->parent = d;

	// d->name = "mappa";
	// d->num_children = 1;
	// d->children[0].f = f;
	// d->parent = root;
	// d->type = 1;

	// root->children[root->num_children++].f = f;

	// vfs_read(f);

	dtree_init();

	pit_init();

	acpi_init(boot_info);

	// if (!timer)
	// pit_init();
	// else
	// 	printk("HPET PIT helyett\n");

	// if (acpi_8042_present())
		ps2_kbd_init();
	// else
		// printk("Nincs PS/2 :(\n");

	tss_init();

	userspace_init();

	asm volatile ("sti");

	dtree_walk();

	for (u32 i = 0; i < dtree[0].h.num_children; i++) {
		dtree_dev* d = dtree_get_child_of_id(0, i);
		// printk("Dev type %s\n", dtree_types[d->h.type]);
		for (u32 j = 0; j < d->h.num_children; j++) {
			dtree_dev* d2 = dtree_get_child(d, j);
			if (d2->h.type == DEV_PCI_MISC) continue;
			// printk("  > Dev type %s: %04x\n", dtree_types[d2->h.type], ((dtree_pci_dev*)d2)->vendor);
			for (u32 k = 0; k < d2->h.num_children; k++) {
				dtree_drive* d3 = dtree_get_child(d2, k);
				// printk("    > Dev type %s\n", dtree_types[d3->h.type]);
				if (d3->h.type != DEV_ATA) continue;
				foreach(p, d3->tbl->num_parts) {
					if (d3->tbl->parts[p].type == PART_ESP) {
						vfs_mkdir("/fat");
						fat32_mount(&d3->tbl->parts[p], "/fat");
					}
				}
			}
		}
	}

	vfs_list_mnts();

	void* content = vmm_alloc();
	fd* f = vfs_open("/fat/tfile");
	vfs_read(f, content, 20);
	printk("content: %s", content);

	printk("dump start\n");
	ramfs_dump_dir((void*)mnts[0].p->drive);
	printk("dump end\n");

	while (1);

	u32 old = con_fg;
	con_fg = 0x0000ff00;
	printk("Kernel idle...");
	con_fg = old;

	khang();
}

_attr_noret void khang() {
	while (1) { asm volatile ("hlt"); }
}
