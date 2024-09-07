#pragma once

#include <acpi/acpi.h>

typedef struct _attr_packed hpet_regs {
	struct _attr_packed {
		u8 rev;
		u8 num_timers : 5;
		u8 capable_64 : 1;
		u8 : 1;
		u8 legacy_replacement : 1;
		u16 vendor;
		u32 period;
	};
	struct _attr_packed {
		u8 enabled : 1;
		u8 legacy_replacement_enabled : 1;
		u64 : 62;
	};
	u64 intr_sts;
	u64 r1[27];
	u64 counter_val;
	u64 : 64;
	struct _attr_packed {
		struct {
			u8 : 1;
			u8 lvl_trig : 1;
			u8 intr : 1;
			u8 periodic : 1;
			u8 periodic_supported : 1;
			u8 timer_64 : 1;
			u8 accumulator : 1;
			u8 : 1;
			u8 force32 : 1;
			u8 gsi : 5;
			u8 fsb_mapping : 1;
			u8 fsb_mapping_supported : 1;
			u16 : 16;
			u32 avl_gsis;
		};
		u64 comparator;
		u64 fsb_intr_route;
	} timers[0];
} hpet_regs;

typedef struct hpet {
	volatile hpet_regs* regs;
} hpet;

extern u8 sleep_done;

void hpet_init(hpet_table* h);
void hpet_start(u64 comparator);
void hpet_stop();
