#pragma once

#include <util/types.h>
#include <util/attrs.h>

#define PIT_CH0 0x40
#define PIT_CMD 0x43

#define PIT_FREQUENCY 1193182

typedef union _attr_packed pit_cmd_reg {
	struct _attr_packed {
		u8 bcd : 1;
		u8 operation : 3;
		u8 access : 2;
		u8 ch : 2;
	};
	u8 raw;
} pit_cmd_reg;

enum {
	PIT_INT_ON_COUNT = 0b000,
	PIT_HW_ONESHOT = 0b001,
	PIT_RATE_GEN = 0b010,
	PIT_SQUARE_WAVE = 0b011,
	PIT_SW_STROBE = 0b100,
	PIT_HW_STROBE = 0b101,
};

enum {
	PIT_LATCH_COUNT_CMD = 0b00,
	PIT_LOBYTE = 0b01,
	PIT_HIBYTE = 0b10,
	PIT_LOHIBYTE = 0b11,
};

extern volatile u64 pit_tick;

void pit_init();
void sleep(u32 time);
