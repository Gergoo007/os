#include <arch/x86/tss.h>
#include <arch/x86/gdt.h>
#include <gfx/console.h>
#include <util/mem.h>
#include <mm/vmm.h>

void tss_init() {
	tss* t = vmm_alloc();

	memset(t, 0, sizeof(tss));

	u64 stack;
	asm volatile ("movq %%rsp, %0" : "=a"(stack));
	t->rsp0 = stack;
	t->bm_offset = sizeof(tss);

	gdt_add_tss(t);

	asm volatile ("ltr %0" :: "a"((u16)0x28));
}
