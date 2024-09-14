#include <mm/pmm.h>
#include <gfx/console.h>
#include <serial/serial.h>

#include <mm/paging.h>

#include <arch/x86/cpuid.h>

u64 heap_base_phys;
u64 heap_base_virt;
u64 heap_size;

bitmap pmm_bm;

u64 pmm_mem_all;
u64 pmm_mem_free;
u64 pmm_mem_used;

void pmm_init(mb_tag* mmap) {
	typedef struct {
		u64 base_addr;
		u64 length;
		u32 type;
		u32 reserved;
	} entry;

	enum {
		MEM_FREE = 1,
		MEM_ACPI = 3,
		MEM_RES = 4,
		MEM_BAD = 5,
	};

	u32 tagsize = mmap->size;
	u32 num_ents = (tagsize - 16) / 24;

	// A preloader az első szabad helyre rakja a kernelt
	u8 firstfree = 1;
	for (entry* e = (entry*)(mmap+2); num_ents; e++, num_ents--) {
		sprintk("%d: %p - %p\n\r", e->type, e->base_addr, e->base_addr+e->length);
		pmm_mem_all += e->length;
		if (e->type == MEM_FREE) {
			pmm_mem_free += e->length;

			if (e->base_addr >= 0x100000000) break;

			if (firstfree || e->base_addr == 0x100000) {
				// if (e->length < 0x30000) {
				// 	firstfree = 0;
				// 	continue;
				// }

				heap_base_phys = e->base_addr + 0x60000;
				heap_size = e->length;
				firstfree = 0;
			}

			if (e->length > heap_size) {
				heap_base_phys = e->base_addr;
				heap_size = e->length;
			}
		}
	}

	// heap_base_phys = 0x40200000;

	sprintk("%d M RAM\n\r", pmm_mem_all >> 20);
	sprintk("Heap @ %p size %d M\n\r", heap_base_phys, heap_size >> 20);

	// Heap mappelése
	if (heap_base_phys < 0x40000000) goto megvan;

megvan:
	heap_base_virt = heap_base_phys | 0xffff800000000000;

	pmm_bm.base = (u8*)heap_base_virt;
	pmm_bm.size = heap_size / 0x1000 / 8 + 1;

	pmm_bm.size |= 0b111;
	pmm_bm.size++;

	bm_init(&pmm_bm);

	// A bitmap-et nem szabad átírni!
	for (u32 i = 0; i < pmm_bm.size / 0x1000 + 1; i += 0x1000) {
		bm_set(&pmm_bm, i, 1);
	}
}

void* pmm_alloc() {
	pmm_mem_free -= 0x1000;
	pmm_mem_used += 0x1000;
	return (void*)heap_base_phys + bm_alloc(&pmm_bm)*0x1000;
}

void pmm_free(void* p) {
	pmm_mem_free += 0x1000;
	pmm_mem_used -= 0x1000;
	bm_set(&pmm_bm, ((u64)p - heap_base_phys) / 0x1000, 0);
}
