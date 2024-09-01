#pragma once

#include <util/types.h>

void printf(void (putc)(char c),
	void (puts)(char* str),
	const char* fmt, ...
);

void vprintf(void (putc)(char c),
	void (puts)(char* str),
	const char* fmt,
	va_list args
);
