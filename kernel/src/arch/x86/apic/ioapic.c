#include <arch/x86/apic/ioapic.h>

u32 ioapic_irqs[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15
};

static dtree_ioapic* find_ioapic(u32 gsi) {
	dtree_for(&dtree[0], i) {
		dtree_ioapic* ch = dtree_get_child(&dtree[0], i);
		if (ch->h.type == DEV_IOAPIC) {
			if (gsi >= ch->gsi_base && gsi < ch->gsi_base+24)
		 		return ch;
		}
	}
	return 0;
}

void ioapic_write(dtree_ioapic* a, u32 reg, u32 val) {
	*(volatile u32*)a->base = reg;
	*(volatile u32*)(a->base + 0x10) = val;
}

u32 ioapic_read(dtree_ioapic* a, u32 reg) {
	*(volatile u32*)a->base = reg;
	return *(volatile u32*)(a->base + 0x10);
}

void ioapic_write_entry(u32 gsi, ioapic_entry entry) {
	dtree_ioapic* a = find_ioapic(gsi);
	ioapic_write(a, 0x10 + gsi * 2, entry.lower);
	ioapic_write(a, 0x11 + gsi * 2, entry.higher);
}

ioapic_entry ioapic_get_entry(u32 gsi) {
	dtree_ioapic* a = find_ioapic(gsi);
	ioapic_entry e;
	e.lower = ioapic_read(a, 0x10 + gsi * 2);
	e.higher = ioapic_read(a, 0x11 + gsi * 2);
	return e;
}

void ioapic_set_mask(u32 gsi, bool mask) {
	dtree_ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.mask = mask;
	ioapic_write(a, 0x10 + gsi * 2, e.lower);
}

void ioapic_set_vector(u32 gsi, u8 v) {
	dtree_ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.vector = v;
	ioapic_write(a, 0x10 + gsi * 2, e.lower);
}

void ioapic_set_destination(u32 gsi, u8 logical, u8 dest) {
	dtree_ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.destination_logical = 0;
	e.destination = 0;
}
