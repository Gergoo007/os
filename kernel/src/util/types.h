#pragma once

#define NULL 0

#ifndef __SIZEOF_INT128__
#error "__uint128_t missing!"
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef __uint128_t u128;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef __int128_t i128;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef i8 int8_t;
typedef i16 int16_t;
typedef i32 int32_t;
typedef i64 int64_t;

typedef u16 wchar;

typedef u64 uintptr_t;

// typedef _Bool bool;

typedef __SIZE_TYPE__ size_t;

#define UINT64_C(v) v##ULL

#define va_start(v, l)	__builtin_va_start(v,l)
#define va_end(v)		__builtin_va_end(v)
#define va_arg(v,l)		__builtin_va_arg(v,l)
#define va_list __builtin_va_list

#define foreach(var, l) for (i64 var = 0; var < (l); var++)
#define offsetof(s, m) __builtin_offsetof(s, m)

#define bytes2kibs(bytes) ((bytes) >> 10)
#define bytes2mibs(bytes) ((bytes) >> 20)
#define bytes2gibs(bytes) ((bytes) >> 30)
#define bytes2tibs(bytes) ((bytes) >> 40)

#define kib2bytes(kibs) ((kibs) << 10)
#define mib2bytes(mibs) ((mibs) << 10)
#define gib2bytes(gibs) ((gibs) << 10)
#define tib2bytes(tibs) ((tibs) << 10)

#define assert(c) if (!(c)) error("Assert failed: %s (%s:%d)", #c, __FILE__, __LINE__)

extern struct elf64_sym* ksymtab;
extern u32 ksymtab_size;
extern void* kstrtab;
extern u32 kstrtab_size;
extern void* kshstrtab;
extern u32 kshstrtab_size;
