#pragma once

#include <util/types.h>

void memset(void* d, u8 v, u64 size);
void memcpy(void* d, const void* s, u64 size);
i32 memcmp(const void* lhs, const void* rhs, size_t count);

void memset_aligned(void* d, u64 v, u64 size);
void memcpy_aligned(void* d, void* s, u64 size);
