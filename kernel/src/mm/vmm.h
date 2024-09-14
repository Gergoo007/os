#pragma once

#include <util/bitmap.h>

typedef struct vmem {
	struct vmem* prev;
	struct vmem* next;
	size_t len;
	u32 sts; // Elég u8 de u32 talán gyorsabb
	u32 padding;
} vmem;

void vmm_init();
void* vmm_alloc();
void vmm_free(void* a);
void* kmalloc(u64 bytes);
void kfree(void* p);
