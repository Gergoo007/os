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

#include <acpi/acpi.h>

#include <boot/multiboot2.h>

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
	// vmm_init();

	// map_page(0xffff880000000000, (u64)pmm_alloc(), 0b11 | MAP_FLAGS_2M);
	// *(u8*)0xffff880000000000 = 0xff;

	// while (1);

	gdt_init();
	idt_init();

	pic_init();

	acpi_init(boot_info);

	asm volatile ("cli");
	while (1) { asm volatile ("hlt"); }
}
