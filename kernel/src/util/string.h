#pragma once

#include <util/types.h>

u8 strlen(const char* str);
u8 wstrlen(const wchar* str);
u32 utf16_to_ascii(wchar* in, char* out);
void utf16_to_asciin(wchar* in, char* out, u32 n);
u32 strcpy(const char* src, char* dest);
u8 strncmp(const char* s1, const char* s2, u32 chars);
u8 strcmp(const char* s1, const char* s2);
void strcat(char* dest, char* src);
void int_to_str(i64 i, char* str);
void uint_to_str(u64 i, char* str);
void uintn_to_str(u64 i, char* str, u8 num);
void hex_to_str(u64 i, char* str);
void hexn_to_str(u64 i, char* str, u8 num);
u64 str_to_uint(const char* s);
