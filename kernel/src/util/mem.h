#pragma once

#include <util/types.h>

void memset(void* d, u8 v, size_t size);
void memcpy(void* d, const void* s, size_t size);
i32 memcmp(const void* lhs, const void* rhs, size_t count);

u64 memcpy_avx_aligned(void* d, void* s, size_t bytes);
