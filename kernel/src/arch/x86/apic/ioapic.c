#include <arch/x86/apic/ioapic.h>

u32 ioapic_irqs[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15
};

static dev_ioapic* find_ioapic(u32 gsi) {
	dev_ioapic* ret = computer->ioapics;
	for (u32 i = 0; i < computer->num_ioapics; i++) {
		if (gsi - computer->ioapics[i].gsi_base < gsi - ret->gsi_base)
			ret = computer->ioapics + i;
	}
	return ret;
}

void ioapic_write(dev_ioapic* a, u32 reg, u32 val) {
	*(volatile u32*)a->hdr.handle = reg;
	*(volatile u32*)(a->hdr.handle + 0x10) = val;
}

u32 ioapic_read(dev_ioapic* a, u32 reg) {
	*(volatile u32*)a->hdr.handle = reg;
	return *(volatile u32*)(a->hdr.handle + 0x10);
}

void ioapic_write_entry(u32 gsi, ioapic_entry entry) {
	dev_ioapic* a = find_ioapic(gsi);
	ioapic_write(a, 0x10 + gsi * 2, entry.lower);
	ioapic_write(a, 0x11 + gsi * 2, entry.higher);
}

ioapic_entry ioapic_get_entry(u32 gsi) {
	dev_ioapic* a = find_ioapic(gsi);
	ioapic_entry e;
	e.lower = ioapic_read(a, 0x10 + gsi * 2);
	e.higher = ioapic_read(a, 0x11 + gsi * 2);
	return e;
}

void ioapic_set_mask(u32 gsi, bool mask) {
	dev_ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.mask = mask;
	ioapic_write(a, 0x10 + gsi * 2, e.lower);
}

void ioapic_set_vector(u32 gsi, u8 v) {
	dev_ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.vector = v;
	ioapic_write(a, 0x10 + gsi * 2, e.lower);
}

void ioapic_set_destination(u32 gsi, u8 logical, u8 dest) {
	dev_ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.destination_logical = 0;
	e.destination = 0;
}
