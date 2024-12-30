#pragma once

#include <util/bitmap.h>

typedef struct vmem {
	struct vmem* prev;
	struct vmem* next;
	size_t len;
	u32 sts; // Elég u8 de u32 talán gyorsabb
	u32 padding;
} vmem;

extern u64 vmm_mem_used;
extern u64 vmm_mem_free;

void vmm_init();
void vmm_dump();
void* vmm_alloc();
void* krealloc(void* p, u64 new);
void vmm_free(void* a);
void* kmalloc(u64 bytes);
void kpremap(void* p);
void kfree(void* p);
