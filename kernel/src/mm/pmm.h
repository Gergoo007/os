#pragma once

#include <util/bitmap.h>
#include <boot/multiboot2.h>

extern u64 pmm_mem_all;
extern u64 pmm_mem_free;
extern u64 pmm_mem_used;

extern u64 heap_base_phys;

void pmm_init(mb_tag* mmap);
void* pmm_alloc();
void pmm_free(void* p);
