#include <util/dynlist.h>
#include <mm/vmm.h>
#include <util/mem.h>

dynlist dynlist_new(u32 inital_size, u32 item_size) {
	dynlist ret = {
		.list = kmalloc(inital_size * item_size),
		.item_size = item_size,
		.size = inital_size,
		.current_count = 0,
	};
	return ret;
}

void dynlist_append(dynlist* list, void* item) {
	if (list->size < list->current_count + 1)
		list->list = krealloc(list->list, list->size*2);
	memcpy(list->list + list->current_count * list->item_size, item, list->item_size);
	list->current_count++;
}

void* dynlist_pop(dynlist* list) {
	return list->list + --list->current_count * list->item_size;
}
