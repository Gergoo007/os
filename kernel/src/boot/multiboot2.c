#include <boot/multiboot2.h>

#include <serial/serial.h>
#include <gfx/framebuffer.h>
#include <gfx/console.h>
#include <mm/pmm.h>

void multiboot2_parse(mb_tag* addr, u64 pl_img_len) {
	addr++;

	while (addr->type) {
		switch (addr->type) {
			case MB_TAG_BOOTLOADER: {
				sprintk("Bootloader: %s\n\r", (u64)addr + 8);
				break;
			}
			case MB_TAG_MMAP: {
				pmm_init(addr, pl_img_len);
				break;
			}
			case MB_TAG_FRAMEBUFFER: {
				mb_fb* fb = (void*)addr;
				// Framebuffer előkészítése
				fb_main.base = FB_VADDR;
				fb_main.size = fb->width * fb->height * (fb->bpp/8);
				fb_main.width = fb->width;
				fb_main.height = fb->height;
				break;
			}
		}

		addr = (void*)((u64)addr + ((addr->size + 7) & ~7));
	}
}
