#pragma once

#include <util/attrs.h>

#if defined (__x86_64__) || defined (_M_X64)

#include <arch/x86/apic/apic.h>
#include <arch/x86/cpu.h>

extern ioapic* ioapics;
extern u32 num_ioapics;
extern u8 ioapic_irqs[16];

extern u64 lapic_base;

#else

#endif

extern cpu* cpus;
extern u32 num_cpus;

void sysinfo_init();
