#include <arch/x86/apic/ioapic.h>
#include <sysinfo.h>

static ioapic* find_ioapic(u32 gsi) {
	for (u32 i = 0; i < num_ioapics; i++) {
		if (gsi >= ioapics[i].gsi_base && gsi < ioapics[i].gsi_base+24)
			return &ioapics[i];
	}
	return 0;
}

void ioapic_write(ioapic* a, u32 reg, u32 val) {
	*(volatile u32*)a->base = reg;
	*(volatile u32*)(a->base + 0x10) = val;
}

u32 ioapic_read(ioapic* a, u32 reg) {
	*(volatile u32*)a->base = reg;
	return *(volatile u32*)(a->base + 0x10);
}

void ioapic_write_entry(u32 gsi, ioapic_entry entry) {
	ioapic* a = find_ioapic(gsi);
	ioapic_write(a, 0x10 + gsi * 2, entry.lower);
	ioapic_write(a, 0x11 + gsi * 2, entry.higher);
}

ioapic_entry ioapic_get_entry(u32 gsi) {
	ioapic* a = find_ioapic(gsi);
	ioapic_entry e;
	e.lower = ioapic_read(a, 0x10 + gsi * 2);
	e.higher = ioapic_read(a, 0x11 + gsi * 2);
	return e;
}

void ioapic_set_mask(u32 gsi, bool mask) {
	ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.mask = mask;
	ioapic_write(a, 0x10 + gsi * 2, e.lower);
}

void ioapic_set_vector(u32 gsi, u8 v) {
	ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.vector = v;
	ioapic_write(a, 0x10 + gsi * 2, e.lower);
}

void ioapic_set_destination(u32 gsi, u8 logical, u8 dest) {
	ioapic* a = find_ioapic(gsi);
	ioapic_entry e = { .lower = ioapic_read(a, 0x10 + gsi * 2) };
	e.destination_logical = 0;
	e.destination = 0;
}
