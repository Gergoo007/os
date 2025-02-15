#pragma once

#include <util/elf.h>
#include <util/attrs.h>
#include <arch/x86/idt.h>

typedef struct _attr_packed process {
	u32 present : 1;
	u32 : 31;
	u32 pcid;
	void* pml4;
	cpu_regs context; // Ebbe van minden regiszter az interrupt létrejöttekor
	u8 pad[48]; // 256 byte-ra
} process;

extern process* proctable;

void sched_init();
void sched_tick();
void sched_save_context(cpu_regs* regs);
u32 sched_new_process_from_elf(void* e);
