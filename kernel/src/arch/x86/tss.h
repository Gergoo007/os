#pragma once

#include <util/types.h>
#include <util/attrs.h>

typedef struct _attr_packed tss {
	u32 : 32;
	u64 rsp0;
	u64 rsp1;
	u64 rsp2;
	u64 : 64;
	u64 : 64;
	u64 ist1;
	u64 ist2;
	u64 ist3;
	u64 ist4;
	u64 ist5;
	u64 ist6;
	u64 ist7;
	u64 : 64;
	u16 : 16;
	u16 bm_offset;
} tss;

void tss_init();
