#pragma once

#include <util/types.h>

void printf(u8 (*putc)(const char* c),
	void (puts)(char* str),
	const char* fmt, ...
);

void vprintf(u8 (*putc)(const char* c),
	void (puts)(char* str),
	const char* fmt,
	va_list args
);
