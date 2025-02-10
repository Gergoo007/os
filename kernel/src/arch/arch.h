#pragma once

#include <util/types.h>

#if defined(__x86_64__) || defined(_M_X64)
__attribute__((always_inline)) static inline void int_en() {
	asm volatile ("sti");
}

__attribute__((always_inline)) static inline void int_dis() {
	asm volatile ("cli");
}

__attribute__((always_inline)) static inline void membar() {
	asm volatile ("pause" ::: "memory");
}
#endif

void sleep(u64 time);
