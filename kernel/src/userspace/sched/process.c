#include <userspace/sched/process.h>
#include <userspace/loader/loader.h>
#include <userspace/init.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <util/mem.h>
#include <arch/x86/apic/apic.h>

#define MAX_PROCS 32768
// Hány ms egy scheduler tick
#define SCHED_TICK 100

// Ez egy táblázat, MAX_PROCS sorral
// Ezen fog végigloopolni mindegyik mag
process* proctable;
u32 num_procs;

// Ez tartja nyomon hogy hányadik processnél tartott a scheduler
u32 sched_counter = 0;

bitmap proc_bm;

// LAPIC timert használva
void sched_init() {
	proctable = kmalloc(MAX_PROCS * sizeof(process));
	memset(proctable, 0, MAX_PROCS * sizeof(process));
	num_procs = 0;

	proc_bm.base = kmalloc(MAX_PROCS / 8 + (MAX_PROCS % 8 ? 1 : 0));
	bm_init(&proc_bm);

	lapic_init_timer(SCHED_TICK);

	// Kernel process (index 0)
	bm_alloc(&proc_bm);

	userinit();
}

// Új address space a jelenlegire mintázva, elf betöltése
u32 sched_new_process_from_elf(void* e) {
	u32 index = bm_alloc(&proc_bm);
	process* p = &proctable[index];

	// Page table-k létrehozása
	// Ami higher half, az minden folyamatban közös
	void* current = pml4;

	pml4->entries[0].addr = 0;

	p->pml4 = pmm_alloc();
	memcpy(VIRTUAL(p->pml4), VIRTUAL(current), 0x1000);

	// A lower halfból minden mehet
	pml4 = VIRTUAL(p->pml4);
	memset(&p->context, 0, sizeof(cpu_regs));
	p->context.rip = (u64)elf_load(e);
	p->context.cs = 0x23;
	p->context.ss = 0x1b;
	p->context.rfl = 0x202;
	void* userstack = pmm_alloc();
	map_page(0x8000000, (u64)userstack, 0b11 | MAP_FLAGS_USER);
	p->context.rsp = 0x8000000 + 0x1000;
	p->context.rbp = 0x8000000 + 0x1000;
	p->pcid = index;
	p->present = 1;
	pml4 = current;

	return index;
}

void sched_save_context(cpu_regs* regs) {
	report("saving for %d\n", sched_counter);
	memcpy(&proctable[sched_counter].context, regs, sizeof(cpu_regs));
	while (!proctable[++sched_counter].present) {
		if (sched_counter >= MAX_PROCS)
			sched_counter = 0;
		else
			sched_counter++;
	}
	report("next %d\n", sched_counter);
}
