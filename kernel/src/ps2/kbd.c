#include <ps2/kbd.h>

#include <arch/x86/apic/apic.h>
#include <arch/arch.h>

// betű - 'a' = index [betű scancode-jához]
static const u8 kbd_set_3[] = {
	// Betűk
	0x1C, 0x32, 0x21, 0x23, 0x24, 0x2B, 0x34, 0x33, 0x43, 0x3B, 0x42, 0x4B, 0x3A,
	0x31, 0x44, 0x4D, 0x15, 0x2D, 0x1B, 0x2C, 0x3C, 0x2A, 0x1D, 0x22, 0x35, 0x1A,

	// Számok
	// In comment: english 0 scancode (key right of 9)
	0x0e, /* 0x45, */ 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46,
};

void ps2_kbd_init() {
	int_dis();

	ioapic_entry e = { .raw = 0, };
	e.vector = 0x40;
	e.mask = 1;
	ioapic_write_entry(ioapic_irqs[1], e);

	inb(PS2_DATA);

	// PS/2 portok kikapcsolása
	outb(0xad, PS2_CMD);
	outb(0xa7, PS2_CMD);

	inb(PS2_DATA);

	// Config byte beállítása
	outb(0x20, PS2_CMD);
	while (!(inb(PS2_STS) & 1));
	// Olvasás
	u8 config_byte = inb(PS2_DATA);
	config_byte |= (1 << 0); // Első port int.
	config_byte &= ~(1 << 1); // Második port int. kikapcs. (egér)
	config_byte |= (1 << 2); // System flag
	config_byte &= ~(1 << 4); // Első port órajel bekapcs.
	config_byte &= ~(1 << 6); // Translation kikapcs.
	// Kiírás
	outb(0x60, PS2_CMD);
	// while (!(inb(PS2_STS) & 1));
	outb(config_byte, PS2_DATA);

	// Self-testet nem csinálok

	// TODO ide

	// Első eszköz bekapcsolása
	outb(0xae, PS2_CMD);
	// while (!(inb(PS2_STS) & 1));

	e.mask = 0;
	ioapic_write_entry(ioapic_irqs[1], e);

	inb(PS2_DATA);

	int_en();
}

char ps2_kbd_convert(u8 scancode) {
	switch (scancode) {
		case 0x66: return '\b';
		case 0x5a: return '\n';
		case 0x29: return ' ';
	}

	for (u8 i = 0; i < sizeof(kbd_set_3); i++) {
		if (kbd_set_3[i] == scancode) {
			if (i > 25)
				return i - 26 + '0';
			else
				return i + 'a';
		}
	}
	sprintk("Kezeletlen scancode: %02x\n\r", scancode);
	return '*';
}
