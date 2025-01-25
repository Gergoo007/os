#include <arch/x86/apic/apic.h>
#include <arch/arch.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <acpi/lai/helpers/sci.h>
#include <mm/paging.h>
#include <mm/vmm.h>

volatile lapic_regs* base;

volatile lapic_regs* lapic_get_base() {
	u32 lo, hi;
	asm volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1b));
	volatile lapic_regs* l = (volatile lapic_regs*)(((u64)lo | ((u64)hi << 32)) & ~0x0fffULL);
	MAKE_VIRTUAL(l);
	return l;
}

void lapic_init() {
	base = lapic_get_base();
	// LAPIC beállítása
	printk("APIC verzió %02x\n", base->version);
	base->spur_int = 0xff | (1 << 8);
}

void lapic_eoi() {
	base->eoi = (u32)0;
}

void apic_process_madt(madt* m) {
	i32 size = m->h.len - (sizeof(madt) - sizeof(m->firstent));
	typeof(&m->firstent) entry = &m->firstent;

	while (size) {
		switch (entry->type) {
			case MADT_IOAPIC: {
				computer->ioapics = krealloc(computer->ioapics, (computer->num_ioapics + 1) * sizeof(dev_ioapic));
				computer->ioapics[computer->num_ioapics].hdr.handle = VIRTUAL((void*)(u64)entry->e_ioapic.ioapic_addr);
				computer->ioapics[computer->num_ioapics].gsi_base = entry->e_ioapic.gsi_base;
				computer->ioapics[computer->num_ioapics].id = entry->e_ioapic.ioapic_id;
				computer->num_ioapics++;
				break;
			}

			case MADT_IOAPIC_OVERRIDE: {
				ioapic_irqs[entry->e_ioapic_overr.irq_src] = entry->e_ioapic_overr.gsi;
				break;
			}

			case MADT_LAPIC: {
				computer->cpus = krealloc(computer->cpus, (computer->num_cpus + 1) * sizeof(dev_cpu));
				computer->cpus[computer->num_cpus].acpi_id = entry->e_lapic.acpi_cpu_id;
				computer->cpus[computer->num_cpus].apic_id = entry->e_lapic.apic_id;
				computer->cpus[computer->num_cpus].online_capable = entry->e_lapic.cpu_online_capable;
				computer->cpus[computer->num_cpus].online = entry->e_lapic.cpu_enabled;
				computer->num_cpus++;
				break;
			}
		}

		size -= entry->len;
		entry = (typeof(entry)) ((u64)entry + entry->len);
		if (size < 0) {
			printk("Majdnem crash gec\n");
			break;
		}
	}

	lapic_init();

	ioapic_entry e = { .raw = 0 };
	e.vector = 0x41;
	ioapic_write_entry(ioapic_irqs[0], e);

	int_en();
}
