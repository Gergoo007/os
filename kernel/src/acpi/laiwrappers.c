#include <mm/vmm.h>
#include <util/mem.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <acpi/lai/core.h>
#include <acpi/acpi.h>
#include <arch/x86/clocks/tsc.h>
#include <util/stacktrace.h>

void laihost_free(void* a, size_t s) {
	kfree(a);
}

void* laihost_malloc(size_t s) {
	// if (!s) s = 0x1;
	return kmalloc(s);
}

void* laihost_realloc(void* old, size_t newsize, size_t oldsize) {
	// if (!newsize) newsize = 0x1;
	if (!oldsize) {
		return kmalloc(newsize);
	}
	return krealloc(old, newsize);
}

_attr_noret void laihost_panic(const char* fmt) {
	u32 old = con_fg;
	con_fg = 0x00ff0000;
	printk(fmt);
	con_fg = old;
	void* rbp;
	asm volatile ("movq %%rbp, %0" : "=r"(rbp));
	stacktrace(0, (void*)rbp);
	while (1);
}

void laihost_log(int lvl, const char* fmt) {
	if (lvl != LAI_DEBUG_LOG && lvl != LAI_WARN_LOG)
		printk("LAI [%d]: %s\n", lvl, fmt);
}

void* laihost_map(size_t address, size_t count) {
	return (void*)(address | 0xffff800000000000);
}

void laihost_unmap(void* pointer, size_t count) {
	return;
}

void laihost_outb(u16 port, u8 val) {
	outb(val, port);
}

void laihost_outw(u16 port, u16 val) {
	outw(val, port);
}

void laihost_outd(u16 port, u32 val) {
	outl(val, port);
}

u8 laihost_inb(u16 port) {
	return inb(port);
}

u16 laihost_inw(u16 port) {
	return inw(port);
}

u32 laihost_ind(u16 port) {
	return inl(port);
}

void laihost_sleep(u64 ms) {
	// tsc_sleep(ms * 1000000);
}

// laihost_pci*: pcie/pcie.c
// laihost_scan: acpi/acpi.c
