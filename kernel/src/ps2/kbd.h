#pragma once

#include <gfx/console.h>

#include <serial/serial.h>

enum {
	PS2_DATA = 0x60,
	PS2_STS = 0x64,
	PS2_CMD = 0x64,
};

enum {
	PS2_KBD_SET_LED = 0xed,
	PS2_KBD_ECHO = 0xee,
	PS2_KBD_SCANCODE = 0xf0,
	PS2_KBD_DETECT = 0xf2,
	PS2_KBD_REPEAT_DELAY = 0xf3,
	PS2_KBD_ENABLE = 0xf4,
	PS2_KBD_DISABLE = 0xf5,
	PS2_KBD_DEFAULTS = 0xf6,
	PS2_KBD_AUTOREP_ONLY = 0xf7,
	PS2_KBD_MAKE_RELEASE = 0xf8,
	PS2_KBD_MAKE_ONLY = 0xf9,
};

void ps2_kbd_init();
char ps2_kbd_convert(u8 scancode);
char ps2_kbd_convert(u8 scancode);
