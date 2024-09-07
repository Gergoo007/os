#include <sysinfo.h>

#include <mm/vmm.h>

#if defined (__x86_64__) || defined (_M_X64)
	ioapic* ioapics;
	u32 num_ioapics;
	u8 ioapic_irqs[16] = { // index: irq, value: gsi
		0,
		1,
		0, // Cascade, általában a PIT-re irányít
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		11,
		12,
		13,
		14,
		15,
	};

	hpet* hpets;
	u32 num_hpets;

	u8 timer; // 0: PIT; 1: HPET
#else

#endif

cpu* cpus;
u32 num_cpus;
u64 lapic_base;

ahci* ahcis;
u32 num_ahcis;

stg_dev* drives;
u32 num_drives;

void sysinfo_init() {
	ioapics = vmm_alloc();
	cpus = vmm_alloc();
	drives = vmm_alloc();
	hpets = vmm_alloc();
}
