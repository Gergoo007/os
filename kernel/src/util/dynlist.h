#pragma once

#include <util/types.h>

typedef struct dynlist {
	void* list;
	u32 item_size;
	u32 size;
	u32 current_count;
} dynlist;

dynlist dynlist_new(u32 inital_size, u32 item_size);
#define dynlist_get(l, index, type) (*(type*)((type)((l)->list + (index) * (l)->item_size)))
void dynlist_append(dynlist* list, void* item);
void* dynlist_pop(dynlist* list);
#define dynlist_foreach(l, type, var) \
	for (type var = (l)->list; (u64)var < (u64)(l)->list + (l)->current_count * (l)->item_size; var += (l)->item_size)
void dynlist_free(dynlist* list);
