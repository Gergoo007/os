#include <util/ustar.h>
#include <util/string.h>
#include <boot/multiboot2.h>

#include <gfx/console.h>

u32 octstr_to_u32(char *str, u32 size) {
	u32 n = 0;
	char* c = str;
	while (size-- > 0) {
		n *= 8;
		n += *c - '0';
		c++;
	}
	return n;
}

tar_file tar_read_file(ustar_hdr* archive, char* name) {
	tar_file ret = {};

	while ((u64)archive < (u64)initrd_end) {
		u32 filesize = octstr_to_u32((char*)archive->size, 11);
		if (!strcmp(name, archive->name)) {
			ret.content = (void*)archive + 512;
			ret.size = filesize;
			return ret;
		}
		if (filesize & 511) filesize = (filesize | 511) + 1;
		archive = ((void*)archive) + (((filesize + 511) / 512) + 1) * 512;
	}

	return ret;
}
