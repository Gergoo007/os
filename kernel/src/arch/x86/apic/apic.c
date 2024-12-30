#include <arch/x86/apic/apic.h>
#include <arch/arch.h>
#include <gfx/console.h>
#include <dtree/tree.h>
#include <serial/serial.h>
#include <acpi/lai/helpers/sci.h>
#include <mm/paging.h>

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
				// printk("ioapic @ %p\n",entry->e_ioapic.ioapic_addr);
				// ioapics[num_ioapics].base = entry->e_ioapic.ioapic_addr | 0xffff800000000000;
				// ioapics[num_ioapics].gsi_base = entry->e_ioapic.gsi_base;
				// ioapics[num_ioapics].id = entry->e_ioapic.ioapic_id;
				// num_ioapics++;
				dtree_ioapic dev = {
					.h.num_children = 0,
					.h.parent = 0,
					.h.type = DEV_IOAPIC,

					.base = (void*)VIRTUAL((u64)entry->e_ioapic.ioapic_addr),
					.gsi_base = entry->e_ioapic.gsi_base,
					.id = entry->e_ioapic.ioapic_id,
				};
				dtree_add_chipset_dev((dtree_dev*)&dev);
				break;
			}

			case MADT_IOAPIC_OVERRIDE: {
				// printk("%d -> %d\n", entry->e_ioapic_overr.gsi, entry->e_ioapic_overr.irq_src);
				ioapic_irqs[entry->e_ioapic_overr.irq_src] = entry->e_ioapic_overr.gsi;
				break;
			}

			case MADT_LAPIC: {
				// lapic_base = m->lapic | 0xffff800000000000;
				// cpus[num_cpus].acpi_id = entry->e_lapic.acpi_cpu_id;
				// cpus[num_cpus].apic_id = entry->e_lapic.apic_id;
				// cpus[num_cpus].capable = entry->e_lapic.cpu_online_capable;
				// cpus[num_cpus].enabled = entry->e_lapic.cpu_enabled;
				// num_cpus++;
				dtree_cpu dev = {
					.h.num_children = 0,
					.h.parent = 0,
					.h.type = DEV_CPU,

					.acpi_id = entry->e_lapic.acpi_cpu_id,
					.apic_id = entry->e_lapic.apic_id,
					.enabled = entry->e_lapic.cpu_enabled,
					.capable = entry->e_lapic.cpu_online_capable,
				};
				dtree_add_chipset_dev((dtree_dev*)&dev);
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
