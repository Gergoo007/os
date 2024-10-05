#include <boot/multiboot2.h>

#include <serial/serial.h>
#include <gfx/framebuffer.h>
#include <gfx/console.h>
#include <mm/pmm.h>

void multiboot2_parse(mb_tag* addr, u64 pl_img_len) {
	addr++;

	while (1) {
		if (addr->type == 0 && addr->size == 8) break;

		switch (addr->type) {
			case MB_TAG_BOOTLOADER: {
				sprintk("Bootloader: %s\n\r", (u64)addr + 8);
				break;
			}
			case MB_TAG_MMAP: {
				pmm_init(addr, pl_img_len);
				break;
			}
		}

		addr = (void*)addr + addr->size;
		if ((u64)addr & 7)
			addr = (mb_tag*)(((u64)addr | 7) + 1);
	}
}
