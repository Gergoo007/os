#include <arch/x86/clocks/pit.h>
#include <arch/arch.h>
#include <serial/serial.h>
#include <gfx/console.h>
#include <arch/x86/apic/ioapic.h>

volatile u64 pit_tick = 0;

void pit_init() {
	int_dis();

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

	ioapic_set_destination(ioapic_irqs[0].gsi, 0, 0);
	ioapic_set_vector(ioapic_irqs[0].gsi, 0x41);
	ioapic_set_mask(ioapic_irqs[0].gsi, 0);

	int_en();
}
