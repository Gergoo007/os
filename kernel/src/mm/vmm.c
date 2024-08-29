#include <mm/vmm.h>
#include <mm/paging.h>
#include <mm/pmm.h>

bitmap vmm_bm;

#include <gfx/console.h>

void vmm_init() {
	// vmm_bm.base = (u8*)0xffff800000000000;
	// // 64 GiB-nyi memóriára elég
	// vmm_bm.size = 0x200000;

	// // Bitmap mappelése, heap többi része mappelve lesz majd amikor kell
	// for (u32 i = 0; i < vmm_bm.size; i += 0x1000) {
	// 	early_map_page(0xffff800000000000 + i, (u64)pmm_alloc(), MAP_KDATA);
	// }

	// bm_init(&vmm_bm);

	// for (u32 i = 0; i < vmm_bm.size / 0x1000; i += 0x1000) {
	// 	bm_set(&vmm_bm, i, 1);
	// }
}

void* vmm_alloc() {
	// void* a = (void*)0xffff800000000000 + bm_alloc(&vmm_bm)*0x1000;
	// // map_page((u64)a, (u64)pmm_alloc(), MAP_KDATA);
	// return a;
	return pmm_alloc() + 0xffff800000000000;
}

// TODO:
void* vmm_reserve() {
	// return (void*)0xffff800000000000 + bm_alloc(&vmm_bm)*0x1000;
	return 0;
}

void vmm_free(void* a) {
	pmm_free(a - 0xffff800000000000);
}
