#include <arch/x86/apic/apic.h>
#include <arch/x86/cpuid.h>
#include <arch/arch.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <acpi/lai/helpers/sci.h>
#include <mm/paging.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>

volatile lapic_regs* lapic;
atomic u32 cpus_up = 1;

#define STACKSIZE 0x1000
void** smp_stacks;

u32 lapic_speed;

volatile lapic_regs* lapic_get_lapic() {
	u32 lo, hi;
	asm volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1b));
	volatile lapic_regs* l = (volatile lapic_regs*)(((u64)lo | ((u64)hi << 32)) & ~0x0fffULL);
	MAKE_VIRTUAL(l);
	return l;
}

void lapic_init() {
	lapic = lapic_get_lapic();
	// LAPIC beállítása
	printk("APIC verzió %02x\n", lapic->version);
	lapic->spur_int = 0xff | (1 << 8);
}

void lapic_eoi() {
	lapic->eoi = (u32)0;
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
				ioapic_irqs[entry->e_ioapic_overr.irq_src].gsi = entry->e_ioapic_overr.gsi;
				ioapic_irqs[entry->e_ioapic_overr.irq_src].active_low = entry->e_ioapic_overr.active_low;
				ioapic_irqs[entry->e_ioapic_overr.irq_src].lvl_trig = entry->e_ioapic_overr.lvl_trig;
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
	// ioapic_write_entry(ioapic_irqs[0], e);

	int_en();
}

// tick: hány ms lapic_speed mennyiségű lapic timer tick
void lapic_init_timer(u32 tick) {
	lapic_lvt timer;
	timer.raw = lapic->lvt_timer;
	timer.tmr.vector = 0x44;
	timer.tmr.mask = 1;
	timer.tmr.mode = LAPIC_PERIODIC;
	lapic->lvt_timer = timer.raw;

	lapic->timer_divide = 0x3;
	lapic->timer_inital = 0xffffffff;
	sleep(tick);
	lapic_speed = 0xffffffff - lapic->timer_current;

	lapic->timer_inital = lapic_speed;

	// A lapic_speed tartalmazza hogy 100 ms alatt mennyit tickelt
	// Most mehetnek az interruptok, periodic módban
	timer.raw = lapic->lvt_timer;
	timer.tmr.mask = 0;
	lapic->lvt_timer = timer.raw;
}

extern void ap_starter();

void lapic_init_smp() {
	// SMP beállítása
	printk("lapic init on bsp; num cores %d\n", computer->num_cpus);

	// Lehet hogy volt valamilyen adat (pl. ACPI) úgyhogy
	// ideiglenesen elpaterolom
	map_page(0x8000, 0x8000, 0b11);
	void* buf = kmalloc(0x1000);
	memcpy(buf, (void*)0x8000, 0x1000);

	u32 bsp = cpu_core();
	memcpy((void*)0x8000, ap_starter, 0x1000);

	u64 cr3;
	asm volatile ("mov %%cr3, %0" : "=a"(cr3));
	printk("cr3 %p\n", cr3);
	*(u32*)0x8500 = cr3;

	smp_stacks = kmalloc(sizeof(void*) * (computer->num_cpus - 1));

	u32 normalamount = 1;

	for (u32 i = 0; i < computer->num_cpus; i++) {
		if (computer->cpus[i].apic_id == bsp) continue;
		// if (!computer->cpus[i].online_capable) continue;
		u32 apic_id = computer->cpus[i].apic_id;

		if (heap_base_phys < 0xa000) warn("Az SMP bootstrap kód veszélyes helyen van!");

		smp_stacks[i-1] = kmalloc(STACKSIZE);

		normalamount++;

		*((volatile uint32_t*)((void*)lapic + 0x280)) = 0;                                                                             // clear APIC errors
		*((volatile uint32_t*)((void*)lapic + 0x310)) = (*((volatile uint32_t*)((void*)lapic + 0x310)) & 0x00ffffff) | (apic_id << 24);         // select AP
		*((volatile uint32_t*)((void*)lapic + 0x300)) = (*((volatile uint32_t*)((void*)lapic + 0x300)) & 0xfff00000) | 0x00C500;          // trigger INIT IPI
		do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)((void*)lapic + 0x300)) & (1 << 12));         // wait for delivery
		*((volatile uint32_t*)((void*)lapic + 0x310)) = (*((volatile uint32_t*)((void*)lapic + 0x310)) & 0x00ffffff) | (apic_id << 24);         // select AP
		*((volatile uint32_t*)((void*)lapic + 0x300)) = (*((volatile uint32_t*)((void*)lapic + 0x300)) & 0xfff00000) | 0x008500;          // deassert
		do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)((void*)lapic + 0x300)) & (1 << 12));         // wait for delivery
		sleep(10);                                                                                                                 // wait 10 msec
		// send STARTUP IPI (twice)
		for(u32 j = 0; j < 1; j++) {
			*((volatile uint32_t*)((void*)lapic + 0x280)) = 0;                                                                     // clear APIC errors
			*((volatile uint32_t*)((void*)lapic + 0x310)) = (*((volatile uint32_t*)((void*)lapic + 0x310)) & 0x00ffffff) | (apic_id << 24); // select AP
			*((volatile uint32_t*)((void*)lapic + 0x300)) = (*((volatile uint32_t*)((void*)lapic + 0x300)) & 0xfff0f800) | 0x000608;  // trigger STARTUP IPI for 0800:0000
			// udelay(200);                                                                                                        // wait 200 usec
			// for (u16 k = 1; k; k++) asm volatile ("nop");
			do { __asm__ __volatile__ ("pause" : : : "memory"); }while(*((volatile uint32_t*)((void*)lapic + 0x300)) & (1 << 12)); // wait for delivery
		}
	}

	kfree(buf);
	// unmap_and_free(0x8000);
	

	while (normalamount > cpus_up);
}

void onsmp() {
	cpus_up++;

	extern gdt_entry* gdt;
	gdt_load(&(gdtr) {
		sizeof(gdt_entry) * 8 - 1,
		(u64) gdt,
	});

	extern idt_entry* idt;
	asm volatile ("lidt %0" :: "m"((idtr) { 0x0fff, idt }));

	while (1) asm volatile ("hlt");
}
