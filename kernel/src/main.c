#include <gfx/console.h>
#include <util/mem.h>
#include <util/string.h>
#include <util/printf.h>
#include <util/dynlist.h>
#include <util/ustar.h>
#include <serial/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>
#include <arch/x86/pic.h>
#include <arch/x86/clocks/pit.h>
#include <arch/x86/clocks/tsc.h>
#include <arch/x86/apic/apic.h>
#include <arch/arch.h>
#include <acpi/lai/core.h>
#include <devmgr/devmgr.h>
#include <ps2/kbd.h>
#include <acpi/acpi.h>
#include <boot/multiboot2.h>
#include <userspace/init.h>
#include <userspace/sched/process.h>
#include <fs/vfs/vfs.h>
#include <fs/gpt.h>
#include <fs/fat32/fat32.h>
#include <fs/vfs/ramfs.h>
#include <modules/modules.h>
#include <storage/ahci/ahci.h>
#include <util/dwarf.h>
#include <usb/hci/xhci.h>

extern _attr_noret void khang();
extern u16* unilookup;

void* initrd_end;
void* initrd;

// TODO: a sleep nem működik (hlt-ot használ, de ahhoz kellenek interruptok)

// ACPI rewrite

// TODO: kfree megjavítása

// TODO: Serial -> arch/x86
// TODO: PS/2: Van-e eszköz az első porton egyáltalán?

// -O3 support

_attr_align_stack _attr_noret void kmain(void* boot_info, u64 preloader_img_len) {
	asm volatile ("mov %%cr3, %0" : "=r"(pml4));

	u64 cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= (1 << 4);
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));

	multiboot2_parse(boot_info, preloader_img_len);

	// TODO: ha igazi hw-en nem jó, ezt nem szabad futtatni
	pic_init();

	con_init();

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

	fb_main.backbuf = kmalloc(fb_main.size);
	memset(fb_main.backbuf, 0, fb_main.size);
	printk("Heap %p\n", heap_base_phys);

	printk("Osszes memoria: %dM\n", pmm_mem_all >> 20);
	printk("Szabad memoria: %dM\n", pmm_mem_free >> 20);

	vfs_init();

	devmgr_init();

	if (!initrd) {
		error("Nincs initrd!");
		while (1);
	}

	// AHCI
	dev_cb_filter filter;
	filter.type = DEV_TRIG_PCI_CLASS_SUBCLASS_PROGIF;
	filter.class.class = 0x1;
	filter.class.subclass = 0x6;
	filter.class.progif = 0x1;
	dev_set_callback(DEV_SIG_CONNECT, filter, ahci_init, 0);

	filter.class.class = 0xc;
	filter.class.subclass = 0x3;
	filter.class.progif = 0x30;
	dev_set_callback(DEV_SIG_CONNECT, filter, xhci_init, 0);

	// PIC maszkok
	outb(0xff, 0xa1);
	outb(0xff, 0x21);

	acpi_init(boot_info);

	pit_init();
	computer->tmr = TMR_PIT;

	lapic_init_smp();

	// if (!timer)
	// pit_init();
	// else
	// 	printk("HPET PIT helyett\n");

	// if (acpi_8042_present())
		ps2_kbd_init();
	// else
	// 	printk("Nincs PS/2 :(\n");

	tss_init();

	userspace_init();

	int_en();

	foreach(i, drives.current_count) {
		dev_drive* d = dynlist_get(&drives, i, dev_drive*);
		foreach(p, d->tbl->num_parts) {
			if (d->tbl->parts[p].type == PART_ESP) {
				// vfs_mkdir("/fat");
				fat32_mount(&d->tbl->parts[p], "/");
			}
		}
	}

	vfs_list_mnts();

	sched_init();

	tar_file f = tar_read_file(initrd, "init");
	if (!f.content) {
		error("No init!");
	} else {
		printk("spawn as %d\n", sched_spawn_elf(f.content));
	}

	u32 old = con_fg;
	con_fg = 0x0000ff00;
	printk("Kernel idle...\n");
	con_fg = old;

	sleep(5000);

	printk("turi ip ip\n");

	if (!f.content) {
		error("No init!");
	} else {
		printk("spawn as %d\n", sched_spawn_elf(f.content));
	}

	khang();
}

// Idle folyamat (PID 0), hogy legyen minek futnia
_attr_noret void khang() {
	while (1) { asm volatile ("hlt"); }
}
