#include <gfx/console.h>
#include <util/string.h>
#include <serial/serial.h>
#include <config.h>

framebuffer fb_main;

u32 con_cx, con_cy;
u32 con_bg, con_fg = 0xffffffff;

void con_init() {
	if (font.magic != 0x864ab572) sprintk("Nem talalni PSF2-t!\n\r");
}

void con_backspace() {

}

void update_cursor() {
	// Betűtípusban hol kezdődik a betű alulról
	u8 _height = 4;

	// Előző elrejtése
	fb_draw_rect(&fb_main, con_cx, con_cy + font.glyph_height - _height, font.glyph_width, 3, con_bg);

	// Új rajzolása
	fb_draw_rect(&fb_main, con_cx + font.glyph_width + 1, con_cy + font.glyph_height - _height, font.glyph_width, 3, con_fg);
}

void cputc(char c) {
	// Karakter létezik?
	if ((u32)c > font.num_glyphs) c = 0;

	// Speciális karakter?
	switch (c) {
		case '\n': {
			for (u8 y = 0; y < font.glyph_height; y++)
				for (u8 x = 0; x < font.glyph_width; x++)
					fb_pixel(fb_main, con_cx + x, con_cy + y, con_bg);

			con_cy += font.glyph_height + 1;
			con_cx = 0;

			fb_draw_rect(&fb_main, con_cx, con_cy + font.glyph_height - 4, font.glyph_width, 3, con_fg);

			return;
		}
		case '\r': {
			for (u8 y = 0; y < font.glyph_height; y++)
				for (u8 x = 0; x < font.glyph_width; x++)
					fb_pixel(fb_main, con_cx + x, con_cy + y, con_bg);
			con_cx = 0;
			return;
		}
		case '\b': {
			if (con_cx < font.glyph_width) return;

			con_cx -= font.glyph_width + 1;
			for (u8 y = 0; y < font.glyph_height; y++)
				for (u8 x = 0; x < font.glyph_width*2+1; x++)
					fb_pixel(fb_main, con_cx + x, con_cy + y, con_bg);

			fb_draw_rect(&fb_main, con_cx, con_cy + font.glyph_height - 4, font.glyph_width, 3, con_fg);

			return;
		}
	}

	if ((u32)con_cx + font.glyph_width*2 + 2 >= fb_main.width) {
		// Sortörés
		for (u8 y = 0; y < font.glyph_height; y++) {
			for (u8 x = 0; x < font.glyph_width; x++) {
				sprintk("@ %d %d\n\r", con_cx + x, con_cy + y);
				fb_pixel(fb_main, con_cx + x, con_cy + y, con_bg);
			}
		}

		con_cy += font.glyph_height + 1;
		con_cx = 0;
	}

	update_cursor();

	// Ha width nem osztható 8-al akkor ki van toldva valamennyi bittel
	u8 padding = 8 - (font.glyph_width % 8);

	u16* glyph = (u16*)((u64)&font + font.size + (c * font.glyph_size));

	for (u32 y = 0; y < font.glyph_height; y++) {
		u16 row = (((*(glyph+y) & 0xff) << 8) | ((*(glyph+y) & 0xff00) >> 8)) >> padding;
		u16 mask = 1 << (font.glyph_width-1);

		for (u32 x = 0; x < font.glyph_width; x++) {
			if (row & mask) {
				fb_pixel(fb_main, con_cx + x, con_cy + y, con_fg);
			} else {
				fb_pixel(fb_main, con_cx + x, con_cy + y, con_bg);
			}
			mask >>= 1;
		}
	}

	con_cx += font.glyph_width+1;
}

void cputs(char* s) {
	while (*s) {
		cputc(*s);
		s++;
	}
}

void _report(const char* fmt, const char* file, ...) {
	if (!sizeof(logmod)) return;
	for (u8 i = 0; i < sizeof(logmod) / sizeof(*logmod); i++) {
		if (!strcmp(file, logmod[i])) break;
		if (i == sizeof(logmod) / sizeof(*logmod) - 1) return;
	}

	u32 old = con_fg;
	con_fg = 0x00fff000;

	va_list args;
	va_start(args, file);

	cputc('[');
	cputs((char*)file);
	cputc(']');
	cputc(' ');

	vprintf(cputc, cputs, fmt, args);

	va_end(args);
	con_fg = old;

	cputc('\n');
}

void _warn(const char* fmt, const char* file, ...) {
	u32 old = con_fg;
	con_fg = 0xff6700;

	va_list args;
	va_start(args, file);
	
	cputc('[');
	cputs((char*)file);
	cputc(']');
	cputc(' ');

	vprintf(cputc, cputs, fmt, args);

	va_end(args);
	con_fg = old;

	cputc('\n');
}

void _error(const char* fmt, const char* file, ...) {
	u32 old = con_fg;
	con_fg = 0x00ff0000;

	va_list args;
	va_start(args, file);

	cputc('[');
	cputs((char*)file);
	cputc(']');
	cputc(' ');

	vprintf(cputc, cputs, fmt, args);

	va_end(args);
	con_fg = old;

	cputc('\n');
}
