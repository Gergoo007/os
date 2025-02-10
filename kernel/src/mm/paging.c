#include <mm/paging.h>
#include <util/mem.h>

#include <gfx/console.h>
#include <arch/x86/cpuid.h>
#include <serial/serial.h>

#include <mm/pmm.h>
#include <mm/vmm.h>

page_table* pml4;

void paging_init(void) {
	// RAM többi részének mappelése
	if (pmm_mem_all < (4ULL<<30)) return;

	u64 remaining_gibs = (pmm_mem_all >> 30) + 1;
	page_table* pdp = (page_table*)(pml4->entries[256].addr & ~0x0fff);
	if (cpu_ps_1g()) {
		for (u64 i = 4; i < remaining_gibs; i++) {
			pdp->entries[i].addr = (i*0x40000000) | 0b10000011;
		}
	} else {
		for (u64 i = 4; i < remaining_gibs; i++) {
			page_table* pd = pmm_alloc();
			memset(pd, 0, 0x200);
			for (u64 j = 0; j < 512; j++) {
				pd->entries[j].addr = (i*0x40000000 + j*0x200000) | 0b11;
			}

			pdp->entries[i].addr = (u64)pd | 0b11;
		}
	}
}

u64 paging_lookup(u64 virt) {
	page_table* cr3;
	asm volatile ("movq %%cr3, %0" : "=r"(cr3));
	MAKE_VIRTUAL(cr3);

	page_table* pdp = (page_table*)(cr3->entries[ADDR_PML4I(virt)].addr & ~0x0fff);
	MAKE_VIRTUAL(pdp);
	page_table* pd = (page_table*)(pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff);
	MAKE_VIRTUAL(pd);
	if (pdp->entries[ADDR_PDPI(virt)].flags & MAP_FLAGS_HUGE) {
		return (u64)pd + (virt & ((1 << 30)-1));
	} else {
		page_table* pt = (page_table*)(pd->entries[ADDR_PDI(virt)].addr & ~0x0fff);
		MAKE_VIRTUAL(pt);
		if (pd->entries[ADDR_PDI(virt)].flags & MAP_FLAGS_HUGE) {
			return PHYSICAL((u64)pt + (virt & 0x1fffff));
		} else {
			return (pt->entries[ADDR_PTI(virt)].addr & ~0x0fff) + (virt & 0x0fff);
		}
	}
}

void map_page(u64 virt, u64 phys, u32 flags) {
	page_table* pdp;
	page_table* pd;
	page_table* pt;

	page_table_entry* entry;

	u8 size;
	if (flags & MAP_FLAGS_2M) {
		phys &= ~((1ULL << 21)-1);
		virt &= ~((1ULL << 21)-1);
		size = 1;
	} else if (flags & MAP_FLAGS_1G) {
		phys &= ~((1ULL << 30)-1);
		virt &= ~((1ULL << 30)-1);
		size = 2;
	} else {
		phys &= ~((1ULL << 12)-1);
		virt &= ~((1ULL << 12)-1);
		size = 0;
	}

	// Custom flagek (bit 12 fölött) eltávolítása
	flags &= (1 << 13)-1;

	entry = &pml4->entries[ADDR_PML4I(virt)];
	if (entry->flags & MAP_FLAGS_PRESENT) {
		pdp = (page_table*) ((pml4->entries[ADDR_PML4I(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
	} else {
		pdp = (page_table*)vmm_alloc();
		memset(pdp, 0, 0x1000);
		pml4->entries[ADDR_PML4I(virt)].addr = (u64)pdp & ~0xffff800000000000ULL;
		pml4->entries[ADDR_PML4I(virt)].flags = flags;
	}

	entry = &pdp->entries[ADDR_PDPI(virt)];
	if (size == 2) { // 1G page
		entry->addr = phys;
		entry->flags = flags | MAP_FLAGS_HUGE;
		return;
	} else {
		if (entry->flags & MAP_FLAGS_PRESENT) {
			pd = (page_table*) ((pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
		} else {
			pd = (page_table*)vmm_alloc();
			memset(pd, 0, 0x1000);
			pdp->entries[ADDR_PDPI(virt)].addr = (u64)pd & ~0xffff800000000000ULL;
			pdp->entries[ADDR_PDPI(virt)].flags = flags;
		}
	}

	entry = &pd->entries[ADDR_PDI(virt)];
	if (size == 1) { // 2M page
		entry->addr = phys;
		entry->flags = flags | MAP_FLAGS_HUGE;
		return;
	} else { // 4K page
		if (entry->flags & MAP_FLAGS_PRESENT) {
			pt = (page_table*) ((pd->entries[ADDR_PDI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
		} else {
			pt = (page_table*)vmm_alloc();
			memset(pt, 0, 0x1000);
			pd->entries[ADDR_PDI(virt)].addr = (u64)pt & ~0xffff800000000000ULL;
			pd->entries[ADDR_PDI(virt)].flags = flags;
		}

		entry = &pt->entries[ADDR_PTI(virt)];
		entry->addr = phys;
		entry->flags = flags;
	}

	asm volatile ("invlpg (%0)" :: "r"(virt));
}
