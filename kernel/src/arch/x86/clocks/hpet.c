#include <arch/x86/clocks/hpet.h>
#include <arch/x86/apic/ioapic.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <mm/paging.h>

u8 sleep_done;

void hpet_init(hpet_table* h) {
	// Periódus: 0x05F5E100 (100 ns)
	// volatile hpet_regs* hr = (volatile hpet_regs*)VIRTUAL(h->addr.address);
	// hpets[num_hpets].regs = hr;
	// num_hpets++;

	// if (h->addr.addr_space != 0) {
	// 	error("IO tér nem támogatott");
	// 	return;
	// }

	// if (!hr->capable_64)
	// 	warn("Nem 64 bites");

	// if (hr->num_timers == 0xff) {
	// 	error("Nincs timer?");
	// 	return;
	// }

	// hr->period = 0x05F5E100;

	// u32 gsi;
	// for (gsi = 16; gsi < 32; gsi++) {
	// 	if (hr->timers[0].avl_gsis & (1 << gsi)) {
	// 		hr->timers[0].gsi = gsi;
	// 		if (hr->timers[0].gsi != gsi)
	// 			continue;
	// 	}
	// }

	// if (gsi == 32) {
	// 	report("Nem sikerült GSI-t találni, legacy lesz");
	// 	if (!hr->legacy_replacement) {
	// 		error("Nincs legacy se (FSB: %d)", hr->timers[0].fsb_mapping_supported);
	// 		return;
	// 	}

	// 	hr->legacy_replacement = 1;
	// 	hr->timers[0].lvl_trig = 0;
	// 	hr->timers[0].comparator = 10000000;
	// 	hr->timers[0].intr = 1;
	// 	hr->timers[0].fsb_mapping = 0;
	// 	hr->timers[0].periodic = 0;
	// 	if (hr->capable_64)
	// 		hr->counter_val = 0;
	// 	else
	// 		*(u32*)&hr->counter_val = 0;

	// 	ioapic_set_vector(ioapic_irqs[0], 0x42);
	// 	ioapic_set_mask(ioapic_irqs[0], 0);

	// 	// hr->enabled = 1;
	// } else {
	// 	hr->legacy_replacement = 0;
	// 	error("dik");
	// }

	// printk("counter %d\n", hr->counter_val);
	// for (u16 i = 1; i; i++);
	// printk("counter %d\n", hr->counter_val);

	// report("HPET megfelel, GSI %d", gsi);
	// timer = 1;
}

// void hpet_start(u64 comparator) {
// 	hpets[0].regs->enabled = 0;
// 	hpets[0].regs->timers[0].intr = 0;
// 	hpets[0].regs->counter_val = 0;
// 	hpets[0].regs->timers[0].comparator = comparator;
// 	hpets[0].regs->timers[0].periodic = 0;
// 	hpets[0].regs->timers[0].force32 = 0;
// 	if (hpets[0].regs->legacy_replacement_enabled)
// 		ioapic_set_mask(ioapic_irqs[0], 0);
// 	else
// 		ioapic_set_mask(hpets[0].regs->timers[0].gsi, 0);
// 	hpets[0].regs->timers[0].intr = 1;
// 	hpets[0].regs->enabled = 1;
// }

// void hpet_stop() {
// 	hpets[0].regs->enabled = 0;
// }
