#include <gfx/console.h>
#include <util/mem.h>
#include <util/string.h>
#include <util/dynlist.h>
#include <serial/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>
#include <arch/x86/pic.h>
#include <arch/x86/clocks/pit.h>
#include <arch/x86/clocks/tsc.h>
#include <devmgr/devmgr.h>
#include <ps2/kbd.h>
#include <acpi/acpi.h>
#include <boot/multiboot2.h>
#include <userspace/init.h>
#include <userspace/loader/loader.h>
#include <fs/vfs/vfs.h>
#include <fs/gpt.h>
#include <fs/fat32/fat32.h>
#include <fs/vfs/ramfs.h>
#include <modules/modules.h>

extern _attr_noret void khang();
extern u16* unilookup;

u32 dump = 0;

// !!!! DYNLIST MIHAMARABB !!!!

// TODO: kfree megjavítása

// TODO: AVX memset
// TODO: Serial -> arch/x86
// TODO: PS/2: Van-e eszköz az első porton egyáltalán?

// Saját ACPI driver??

void oncon() {
	printk("LE LETT HÍVVA!\n");
}

_attr_align_stack _attr_noret void kmain(void* boot_info, u64 preloader_img_len) {
	// u8 src[128];
	// u8 des[128];
	// memcpy(des, src, 24);
	// while (1);
	// Rendes hardwaren ez befagyasztja a rendszert...
	// serial_init(0x3f8);

	// asm volatile ("outw %0, %1" :: "a"((u16)0x8A00), "d"((u16)0x8A00));

	asm volatile ("mov %%cr3, %0" : "=r"(pml4));

	u64 cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= (1 << 4);
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));

	sprintk("teszt\n\r");

	multiboot2_parse(boot_info, preloader_img_len);

	// TODO: ha igazi hw-en nem jó, ezt nem szabad futtatni
	pic_init();

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

	devmgr_init();
	dev_cb_filter filter;
	filter.type = DEV_TRIG_PCI_VENDOR_PRODUCT;
	filter.vp.vendor = 0x8086; // AHCI kontroller
	filter.vp.product = 0x2922;
	dev_set_callback(DEV_SIG_CONNECT, filter, oncon, 0);

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

	for (u32 i = 0; i < num_drives; i++) {
		printk("drive type %s\n", dev_types[drives[i].hdr.type]);
	}

	// for (u32 i = 0; i < dtree[0].h.num_children; i++) {
	// 	dtree_dev* d = dtree_get_child_of_id(0, i);
	// 	// printk("Dev type %s\n", dtree_types[d->h.type]);
	// 	for (u32 j = 0; j < d->h.num_children; j++) {
	// 		dtree_dev* d2 = dtree_get_child(d, j);
	// 		if (d2->h.type == DEV_PCI_MISC) continue;
	// 		// printk("  > Dev type %s: %04x\n", dtree_types[d2->h.type], ((dtree_pci_dev*)d2)->vendor);
	// 		for (u32 k = 0; k < d2->h.num_children; k++) {
	// 			dtree_drive* d3 = dtree_get_child(d2, k);
	// 			// printk("    > Dev type %s\n", dtree_types[d3->h.type]);
	// 			if (d3->h.type != DEV_ATA) continue;

	// 			foreach(p, d3->tbl->num_parts) {
	// 				if (d3->tbl->parts[p].type == PART_ESP) {
	// 					vfs_mkdir("/fat");
	// 					fat32_mount(&d3->tbl->parts[p], "/fat");
	// 				}
	// 			}
	// 		}
	// 	}
	// }

	vfs_list_mnts();
	dd* fatroot = vfs_readdir("/fat");
	printk("fileok: ");
	for (u32 i = 0; i < fatroot->num_entries; i++) {
		printk("%s ", fatroot->entries[i].name);
		char buf[256];
		strcat(buf, "/fat");
		strcat(buf, fatroot->entries[i].name);
		dd* asd = vfs_readdir(buf);
		for (u32 j = 0; j < asd->num_entries; j++) {
			printk("%s ", asd->entries[j].name);
		}
	}
	printk("\n");

	u32 old = con_fg;
	con_fg = 0x0000ff00;
	printk("Kernel idle...");
	con_fg = old;

	khang();
}

_attr_noret void khang() {
	while (1) { asm volatile ("hlt"); }
}
