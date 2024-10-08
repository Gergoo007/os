#pragma once

#include <util/types.h>

static inline void int_en() {
	asm volatile ("sti");
}

static inline void int_dis() {
	asm volatile ("cli");
}

void sleep(u64 time);
