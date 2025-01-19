#include <util/string.h>

// Szöveg hossza a null terminátor nélkül
u8 strlen(const char* str) {
	u8 len = 0;
	while (*(str++))
		len++;

	return len;
}

// Szöveg hossza de a szöveg UTF-16/UCS-2
u8 wstrlen(const wchar* str) {
	u8 len = 0;
	while (*(str++))
		len++;

	return len;
}

u32 utf16_to_ascii(wchar* in, char* out) {
	u32 count = 0;
	while (in[count]) {
		out[count] = in[count];
		count++;
	}
	out[count] = 0;
	return count;
}

void utf16_to_asciin(wchar* in, char* out, u32 n) {
	out[n] = '\0';
	while (n--) {
		*out = *in;
		out++;
		in++;
	}
}

u32 strcpy(const char* src, char* dest) {
	u32 len = 0;
	while (src[len]) {
		// *dest = *src;
		// src++;
		// dest++;
		dest[len] = src[len];
		len++;
	}
	// *dest = '\0';
	dest[len]= 0;
	return len;
}

u8 strncmp(const char* s1, const char* s2, u32 chars) {
	while (chars-- && *s1 && *s2) {
		if (*s1 != *s2)
			return 1;
		s1++;
		s2++;
	}
	return 0;
}

u8 strcmp(const char* s1, const char* s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2)
			return 1;
		s1++;
		s2++;
	}
	if (*s1 != *s2) return 1;
	return 0;
}

void strcat(char* dest, char* src) {
	while (*dest) dest++;

	while (*src) {
		*dest = *src;

		src++;
		dest++;
	}
	*dest = 0;
}

void int_to_str(i64 i, char* str) {
	u8 negative = (i < 0) ? 1 : 0;
	u8 len = 1;

	if (negative)
		i *= -1;

	// Ha nulla, akkor egyszerű a dolgunk
	// Ugyanakkor ez bypassel egy buggot:
	// mivel a szám 0, a length is 0 lesz, ezért üres stringet kapunk
	if (!i) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; i > 0; j++, i /= 10) {
		str[j] = (i % 10) + '0';
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';

	if (negative) {
		// A kötőjelet oda kell rakni ha negatív
		char tmp[256];
		tmp[0] = '-';
		strcpy(str, tmp+1);
		strcpy(tmp, str);
	}
}

void uint_to_str(u64 i, char* str) {
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; i > 0; j++, i /= 10) {
		str[j] = (i % 10) + '0';
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';
}

void uintn_to_str(u64 i, char* str, u8 num) {
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; j < num; j++, i /= 10) {
		str[j] = (i % 10) + '0';
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';
}

void hex_to_str(u64 i, char* str) {
	if (i == 0) {
		str[0] = '0';
		str[1] = 0;
		return;
	}

	const char* numbers = "0123456789abcdef";
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; i > 0; j++, i /= 16) {
		str[j] = *((i % 16) + numbers);
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';
}

void hexn_to_str(u64 i, char* str, u8 num) {
	const char* numbers = "0123456789abcdef";
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; j < num; j++, i /= 16) {
		str[j] = *((i % 16) + numbers);
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';
}

u64 str_to_uint(const char* s) {
	u64 ret = 0;
	while (*s >= '0' && *s <= '9') {
		ret *= 10;
		ret += *s - '0';

		s++;
	}
	return ret;
}
