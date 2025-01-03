#pragma once

#include <util/elf.h>

typedef struct module {
	void (*entry)();
} module;

extern u32 num_modules;
extern module* modules;

u32 module_link(void* m);
