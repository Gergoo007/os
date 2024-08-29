#pragma once

#include <util/types.h>
#include <util/attrs.h>

_attr_unused static inline
void cpuid(u32 eax, u32 ecx, u32* a, u32* b, u32* c, u32* d) {
	asm volatile (
		"cpuid" :
		"=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) :
		"a"(eax), "c"(ecx) :
	);
}

_attr_unused static inline
u32 cpu_core() {
	u32 ebx;
	asm volatile (
		"cpuid" :
		"=b"(ebx) :
		"a"(1) :
		"ecx", "edx"
	);
	return (ebx >> 24) & 0xffff;
}

_attr_unused static inline
void cpu_vendor(char* vendor) {
	asm volatile (
		"cpuid" :
		"=b"(*(u32*)vendor), "=d"(*((u32*)vendor+1)), "=c"(*((u32*)vendor+2)) :
		"a"(0), "c"(0) :
	);
}

_attr_unused static inline
bool cpu_ps_1g() {
	u32 edx;
	asm volatile (
		"cpuid" :
		"=d"(edx) :
		"a"(0x80000001), "c"(0) :
		"ebx"
	);
	return edx & (1 << 26);
}
