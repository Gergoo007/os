#include <arch/x86/apic/apic.h>
#include <gfx/console.h>
#include <sysinfo.h>
#include <serial/serial.h>
#include <acpi/lai/helpers/sci.h>

void lapic_init() {
	volatile lapic_regs* l = (volatile lapic_regs*)lapic_base;
	// LAPIC beállítása
	printk("APIC verzio: %02x\n", l->version);
	l->spur_int = 0xff | (1 << 8);
}

void lapic_eoi() {
	// asm volatile ("outw %0, %1" :: "a"((u16)0x8AE0), "d"((u16)0x8A00));
	volatile lapic_regs* l = (volatile lapic_regs*)lapic_base;
	l->eoi = (u32)0;
}

void apic_process_madt(madt* m) {
	i32 size = m->h.len - (sizeof(madt) - sizeof(m->firstent));
	typeof(&m->firstent) entry = &m->firstent;

	while (size) {
		switch (entry->type) {
			case MADT_IOAPIC: {
				// printk("ioapic @ %p\n",entry->e_ioapic.ioapic_addr);
				ioapics[num_ioapics].base = entry->e_ioapic.ioapic_addr | 0xffff800000000000;
				ioapics[num_ioapics].gsi_base = entry->e_ioapic.gsi_base;
				ioapics[num_ioapics].id = entry->e_ioapic.ioapic_id;
				num_ioapics++;
				break;
			}

			case MADT_IOAPIC_OVERRIDE: {
				// printk("%d -> %d\n", entry->e_ioapic_overr.gsi, entry->e_ioapic_overr.irq_src);
				ioapic_irqs[entry->e_ioapic_overr.irq_src] = entry->e_ioapic_overr.gsi;
				break;
			}

			case MADT_LAPIC: {
				lapic_base = m->lapic | 0xffff800000000000;
				cpus[num_cpus].acpi_id = entry->e_lapic.acpi_cpu_id;
				cpus[num_cpus].apic_id = entry->e_lapic.apic_id;
				cpus[num_cpus].capable = entry->e_lapic.cpu_online_capable;
				cpus[num_cpus].enabled = entry->e_lapic.cpu_enabled;
				num_cpus++;
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

	// TODO: normális rendszer interruptokra pl. PIC fallback
	if (num_ioapics) {
		ioapic_entry e = { .raw = 0 };
		e.vector = 0x41;
		ioapic_write_entry(ioapic_irqs[0], e);
	}

	sti();
}
