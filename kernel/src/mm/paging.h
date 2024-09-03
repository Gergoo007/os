#pragma once

#include <util/types.h>
#include <util/attrs.h>

enum {
	// Saját flagek a 2M és 1G-osok megkülönböztetésére
	MAP_FLAGS_1G	=	(1 << 31),
	MAP_FLAGS_2M	=	(1 << 30),

	MAP_FLAGS_HUGE =	0b10000000,
	MAP_FLAGS_USER = 	0b00000100,
	MAP_FLAGS_RW =		0b00000010,
	MAP_FLAGS_PRESENT =	0b00000001,

	MAP_UDATA = MAP_FLAGS_USER | MAP_FLAGS_RW | MAP_FLAGS_PRESENT,
	MAP_KDATA = MAP_FLAGS_RW | MAP_FLAGS_PRESENT,
};

#define ADDR_PTI(a) ((((u64)a) >> 12ULL) & 511ULL)
#define ADDR_PDI(a) ((((u64)a) >> 21ULL) & 511ULL)
#define ADDR_PDPI(a) ((((u64)a) >> 30ULL) & 511ULL)
#define ADDR_PML4I(a) ((((u64)a) >> 39ULL) & 511ULL)

#define MAKE_VIRTUAL(addr) (addr = (typeof(addr))((u64)addr | 0xffff800000000000))
#define VIRTUAL(addr) ((typeof(addr)) ((u64)addr | 0xffff800000000000))
#define PHYSICAL(addr) ((typeof(addr)) ((u64)addr & ~0xffff800000000000))
#define MAKE_PHYSICAL(addr) (addr = (typeof(addr))((u64)addr & ~0xffff800000000000))

typedef struct _attr_packed page_table_entry {
	union {
		u16 flags : 12;
		u64 addr;
	};
} page_table_entry;

typedef struct _attr_packed page_table {
	page_table_entry entries[512];
} page_table;

extern page_table* pml4;

void paging_init(void);
u64 paging_lookup(u64 virt);
void map_page(u64 virt, u64 phys, u32 flags);
