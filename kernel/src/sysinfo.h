#pragma once

#include <util/attrs.h>

#if defined (__x86_64__) || defined (_M_X64)

#include <arch/x86/apic/apic.h>
#include <arch/x86/cpu.h>
#include <arch/x86/clocks/hpet.h>

extern ioapic* ioapics;
extern u32 num_ioapics;
extern u8 ioapic_irqs[16];

extern u64 lapic_base;

extern hpet* hpets;
extern u32 num_hpets;

extern u8 timer; // 0: PIT; 1: HPET
extern u8 pm_timer_present;

#else

#endif

extern cpu* cpus;
extern u32 num_cpus;

#include <storage/storage.h>
#include <storage/ahci/ahci.h>

#include <usb/hci/uhci.h>

extern stg_dev* drives;
extern u32 num_drives;

extern uhci* uhcis;
extern u32 num_uhcis;

void sysinfo_init();
