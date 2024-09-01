#pragma once

#include <gfx/framebuffer.h>
#include <util/printf.h>

typedef struct psf2_hdr {
	u32 magic; // 0x72b54a86
	u32 ver;
	u32 size;
	u32 flags;
	u32 num_glyphs;
	u32 glyph_size;
	u32 glyph_height;
	u32 glyph_width;
} psf2_hdr;

extern psf2_hdr _binary_src_font_psf_start;
#define font _binary_src_font_psf_start

extern u8 _binary_src_font_psf_end;
#define fontend _binary_src_font_psf_end

#define printk(fmt, ...) printf(cputc, cputs, fmt, ##__VA_ARGS__)

#define report(f, ...) _report(f, __FILE_NAME__, ##__VA_ARGS__)
#define warn(f, ...) _warn(f, __FILE_NAME__, ##__VA_ARGS__)
#define error(f, ...) _error(f, __FILE_NAME__, ##__VA_ARGS__)

void con_init();
void cputc(const char c);
void cputs(char* s);
void _report(const char* fmt, const char* file, ...);
void _warn(const char* fmt, const char* file, ...);
void _error(const char* fmt, const char* file, ...);
