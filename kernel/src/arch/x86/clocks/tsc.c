#include <arch/x86/cpuid.h>
#include <gfx/console.h>
#include <arch/x86/clocks/tsc.h>
#include <acpi/acpi.h>

u64 tsc_ticks_per_ns;

void tsc_configure() {
	// u32 eax, ebx, ecx, edx;
	// cpuid(0x15, 0, &eax, &ebx, &ecx, &edx);
	// if (!ebx) {
	// 	warn("Hülye a TSC CPUID");
	// 	return;
	// }

	// tsc_freq = ecx * ebx / eax;
	// printk("perido %d us; freq %d\n", 1000000 / tsc_freq, tsc_freq);

	// u64 start = rdtsc();
	// printk("diff %p\n", rdtsc() - start);

	// cpuid(0x80000007, 0, &eax, &ebx, &ecx, &edx);
	// if (!(edx & (1 << 8)))
	// 	warn("TSC frekv. nem állandó!");

	// printk("sleep start\n");
	// tsc_sleep(50000000000);
	// printk("sleep end\n");
}

u64 rdtsc() {
	u64 rdx, rax;
	asm volatile ("rdtsc" : "=d"(rdx), "=a"(rax));
	return (rdx << 32) | rax;
}

void tsc_configure_using_acpi() {
	u32 eax, ebx, ecx, edx;
	cpuid(0x80000007, 0, &eax, &ebx, &ecx, &edx);
	if (!(edx & (1 << 8)))
		warn("TSC frekv. nem állandó!");

	u64 astart = acpi_read_timer();
	u64 tstart = rdtsc();

	for (u16 i = 1; i; i++) asm volatile ("nop");

	u64 tend = rdtsc();
	u64 aend = acpi_read_timer();

	u64 nanosecs = (aend - astart) * 279; // 279,36 kéne hogy legyen
	tsc_ticks_per_ns = (tend - tstart) / nanosecs;
}

void tsc_sleep(u64 ns) {
	u64 start = rdtsc();
	u64 target = start + (ns * tsc_ticks_per_ns);
	while (rdtsc() < target);
}
