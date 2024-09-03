#pragma once

#include <util/attrs.h>
#include <util/types.h>

#include <arch/x86/tss.h>

enum {
	GDT_LDT = 2,
	GDT_TSS_AVL = 9,
	GDT_TSS_BUSY = 11,
	GDT_CALL_GATE = 12,
	GDT_INT_GATE = 14,
	GDT_TRAP_GATE = 15,
};

typedef struct _attr_packed gdt_entry {
	u16 limit1;
	u16 base1;
	u8 base2;
	union {
		struct { // access byte
			u8 accessed : 1;
			u8 rw : 1;
			u8 conforming : 1; // Ha 0, a kernel nem futtathat user kódot
			u8 exec : 1;
			u8 data_code : 1; // Ha 0, system segment
			u8 dpl : 2;
			u8 present : 1;
		} seg_desc;
		struct { // access byte
			u8 type : 4;
			u8 data_code : 1; // Ha 0, system segment
			u8 dpl : 2;
			u8 present : 1;
		} sys_seg_desc;
	} access_byte;
	u8 limit2 : 4;
	u8 avl : 1;
	u8 long_mode : 1;
	u8 size : 1;
	u8 limit_in_pages : 1; // granularity
	u8 base3;
} gdt_entry;

typedef struct _attr_packed gdtr {
	u16 size;
	u64 base;
} gdtr;

void gdt_init();
void gdt_load(gdtr*);
void gdt_add_tss(tss* t);
