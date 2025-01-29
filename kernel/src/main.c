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
#include <storage/ahci/ahci.h>

extern _attr_noret void khang();
extern u16* unilookup;

u32 dump = 0;

// ACPI rewrite

// TODO: kfree megjavítása

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

	// AHCI
	dev_cb_filter filter;
	filter.type = DEV_TRIG_PCI_VENDOR_PRODUCT;
	filter.vp.vendor = 0x8086; // AHCI kontroller
	filter.vp.product = 0x2922;
	dev_set_callback(DEV_SIG_CONNECT, filter, ahci_init, 0);

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

	dev_drive* d = dynlist_get(&drives, 0, dev_drive*);
	foreach(p, d->tbl->num_parts) {
		if (d->tbl->parts[p].type == PART_ESP) {
			vfs_mkdir("/fat");
			fat32_mount(&d->tbl->parts[p], "/fat");
		}
	}

	vfs_list_mnts();

	u32 old = con_fg;
	con_fg = 0x0000ff00;
	printk("Kernel idle...");
	con_fg = old;

	khang();
}

_attr_noret void khang() {
	while (1) { asm volatile ("hlt"); }
}
