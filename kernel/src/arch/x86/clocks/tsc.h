#pragma once

#include <util/types.h>

void tsc_configure();
void tsc_configure_using_acpi();
u64 rdtsc();
void tsc_sleep(u64 ns);
