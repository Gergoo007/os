#pragma once

#include <util/types.h>
#include <util/attrs.h>

typedef struct cpu {
	u32 apic_id;
	u8 acpi_id;
	u8 enabled : 1;
	u8 capable : 1;
} cpu;

_attr_unused static inline void cli() { asm volatile ("cli"); }
_attr_unused static inline void sti() { asm volatile ("sti"); }
