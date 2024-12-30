#pragma once

#include <util/elf.h>
#include <mm/vmm.h>
#include <gfx/console.h>

void (*elf_load(void* elf))();
