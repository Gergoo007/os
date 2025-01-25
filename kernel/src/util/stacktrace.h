#pragma once

#include <util/elf.h>

typedef struct stackframe {
	struct stackframe* rbp;
	u64 rip;
} stackframe;

void stacktrace(u64 currentaddr, stackframe* rbp);
