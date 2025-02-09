#include <gfx/framebuffer.h>
#include <util/mem.h>

void fb_draw_rect(framebuffer* fb, u32 x, u32 y, u32 w, u32 h, u32 c) {
	for (u32 i = 0; i < w; i++) {
		for (u32 j = 0; j < h; j++) {
			// *(u32*)(fb.base + ((i+x)*(4)) + ((j+y)*(4)*fb.width)) = c;
			fb_pixel((*fb), x+i, y+j, c);
		}
	}
}

void fb_draw_rect_direct(framebuffer* fb, u32 x, u32 y, u32 w, u32 h, u32 c) {
	for (u32 i = 0; i < w; i++) {
		for (u32 j = 0; j < h; j++) {
			// *(u32*)(fb.base + ((i+x)*(4)) + ((j+y)*(4)*fb.width)) = c;
			fb_pixel_direct((*fb), x+i, y+j, c);
		}
	}
}

void fb_swap(framebuffer* fb) {
	if (fb->backbuf == (void*)fb->base) return;

	memcpy_avx_aligned((void*)fb->base, fb->backbuf, fb->width*fb->height*4);
}
