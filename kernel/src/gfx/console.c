#include <gfx/console.h>
#include <util/string.h>
#include <util/mem.h>
#include <serial/serial.h>
#include <config.h>
#include <mm/pmm.h>

framebuffer fb_main;

u32 con_cx, con_cy;
u32 con_bg, con_fg = 0xffffffff;

u16* unilookup;

void con_init() {
	if (font.magic != 0x864ab572) sprintk("Nem talalni PSF2-t!\n\r");

	unilookup = pmm_alloc();
	for (u64 i = (u64)unilookup; i < (u64)unilookup + 0xffff; i += 0x1000) {
		if (i != (u64)pmm_alloc()-0x1000) sprintk("unilookup allocation hiba\n\r");
	}
	memset(unilookup, 0, 0x1000);

	// Unicode lookup táblázat kitöltése
	u8* s = (u8*)((u64)&font + sizeof(psf2_hdr) + font.glyph_size*font.num_glyphs);
	u16 glyph = 0;
	while (s < &_binary_src_font_psf_end) {
		u16 uc = (u16)s[0];
		if(uc == 0xFF) {
			glyph++;
			s++;
			continue;
		} else if(uc & 128) {
			/* UTF-8 to unicode */
			if((uc & 32) == 0 ) {
				uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
				s++;
			} else
			if((uc & 16) == 0 ) {
				uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
				s+=2;
			} else
			if((uc & 8) == 0 ) {
				uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
				s+=3;
			} else
				uc = 0;
		}
		/* save translation */
		unilookup[uc] = glyph;
		s++;
	}
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

void cputglyph(u16 glyphnum) {
	// Ha width nem osztható 8-al akkor ki van toldva valamennyi bittel
	u8 padding = 8 - (font.glyph_width % 8);
	u16* glyph = (u16*)((u64)&font + font.size + (glyphnum * font.glyph_size));

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

	cputglyph(c);
}

u8 cputu8(const char* unichar) {
	if (!(*(u8*)unichar & 0x80)) {
		cputc(*unichar);
		return 1;
	}

	if (!(font.flags & 1)) {
		cputc(' ');
		return 1;
	}

	u32 value = 0;
	u8 len;
	if ((*unichar & 0b11110000) == 0b11110000) {
		len = 4;
		value |= (((u32)unichar[0] & 0b00000111) << 18);
		value |= (((u32)unichar[1] & 0b00111111) << 12);
		value |= (((u32)unichar[2] & 0b00111111) << 6);
		value |= (((u32)unichar[3] & 0b00111111));
	} else if ((*unichar & 0b11100000) == 0b11100000) {
		len = 3;
		value |= (((u32)unichar[0] & 0b00001111) << 12);
		value |= (((u32)unichar[1] & 0b00111111) << 6);
		value |= (((u32)unichar[2] & 0b00111111));
	} else {
		len = 2;
		value |= (((u32)unichar[0] & 0b00011111) << 6);
		value |= (((u32)unichar[1] & 0b00111111));
	}

	cputglyph(unilookup[value]);
	return len;
}

void cputs(char* s) {
	while (*s)
		s += cputu8(s);
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

	vprintf(cputu8, cputs, fmt, args);

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

	vprintf(cputu8, cputs, fmt, args);

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

	vprintf(cputu8, cputs, fmt, args);

	va_end(args);
	con_fg = old;

	cputc('\n');
}
