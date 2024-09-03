#include <gfx/console.h>
#include <util/mem.h>
#include <serial/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/paging.h>
#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>
#include <arch/x86/pic.h>
#include <arch/x86/cpuid.h>
#include <ps2/kbd.h>
#include <acpi/acpi.h>
#include <boot/multiboot2.h>
#include <sysinfo.h>
#include <userspace/init.h>

extern _attr_noret void khang();

_attr_noret void kmain(void* boot_info) {
	asm volatile ("mov %%cr3, %0" : "=r"(pml4));

	u64 cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= (1 << 4);
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));

	// Framebuffer előkészítése
	fb_main.base = FB_VADDR;
	fb_main.size = 1024*768*4;
	fb_main.width = 1024;
	fb_main.height = 768;
	con_init();

	multiboot2_parse(boot_info);

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

	vmm_init();

	gdt_init();
	idt_init();

	pic_init();

	sysinfo_init();

	acpi_init(boot_info);

	ps2_kbd_init();
	tss_init();

	userspace_init();

	// *(u8*)0xfffffffffff00000 = 0xff;

	// foreach(i, num_cpus) {
	// 	printk("core/thread %d (acpi %d); r %d c %d\n", cpus[i].apic_id, cpus[i].acpi_id, cpus[i].enabled, cpus[i].capable);
	// }

	// foreach(i, num_drives) {
	// 	printk("meghajto: %s\n", drives[i].model);
	// }

	khang();
}

_attr_noret void khang() {
	asm volatile ("sti");
	while (1) { asm volatile ("hlt"); }
}
