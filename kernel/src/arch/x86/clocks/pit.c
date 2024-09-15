#include <arch/x86/clocks/pit.h>
#include <arch/x86/cpu.h>
#include <serial/serial.h>
#include <sysinfo.h>
#include <gfx/console.h>

volatile u64 pit_tick = 0;

void pit_init() {
	cli();

	pit_cmd_reg cmd = {
		.bcd = 0,
		.operation = PIT_RATE_GEN,
		.access = PIT_LOHIBYTE,
		.ch = 0,
	};

	outb(cmd.raw, PIT_CMD);

	// Egy 'katt' 1 ms
	u16 count = 1250;

	outb(count & 0x00ff, PIT_CH0);
	outb((count & 0xff00) >> 8, PIT_CH0);

	// ioapic_set_destination(ioapic_irqs[0], 0, 0);
	// ioapic_set_vector(ioapic_irqs[0], 0x41);
	// ioapic_set_mask(ioapic_irqs[0], 0);

	ioapic_entry e = { .raw = 0, };
	e.vector = 0x41;
	ioapic_write_entry(ioapic_irqs[0], e);

	sti();
}
