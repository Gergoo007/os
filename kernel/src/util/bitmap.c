#include <util/bitmap.h>
#include <util/mem.h>

void bm_init(bitmap* bm) {
	memset(bm->base, 0, bm->size);
}

u64 bm_alloc(bitmap* bm) {
	for (u64 quad = 0; quad < (bm->size/8)+1; quad++) {
		for (u32 i = 0; i < 64; i++) {
			if (*((u64*)bm->base+quad) & (1ULL << i)) continue;
			bm_set(bm, quad*64 + i, 1);
			return quad*64 + i;
		}
	}
	return -1;
}

void bm_set(bitmap* bm, u64 index, u8 val) {
	if (val)
		bm->base[index / 8] |= (1 << (index & 7));
	else
		bm->base[index / 8] &= ~(1 << (index & 7));
}

u8 bm_get(bitmap* bm, u64 index) {
	return bm->base[index/8] & (1 << (index&7));
}
