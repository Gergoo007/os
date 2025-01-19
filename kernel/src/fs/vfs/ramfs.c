#include <fs/vfs/ramfs.h>
#include <util/string.h>
#include <util/mem.h>
#include <mm/vmm.h>
#include <gfx/console.h>

void ramfs_register_module() {
	fs_vtables[FSTYPE_RAMFS] = (fs_vtable) {
		.fs_mount = ramfs_mount,
		.fs_read = ramfs_read,
		.fs_readdir = ramfs_readdir,
		.fs_write = ramfs_write,
		.fs_create = ramfs_create,
		.fs_mkdir = ramfs_mkdir,
		.fs_get_size = ramfs_get_size,
	};
}

u32 depth = 0;
void ramfs_dump_dir(ramfs_dir* dir) {
	// if (dir->name[0] == '\0') printk("/\n");
	for (u32 i = 0; i < dir->num_entries; i++) {
		depth++;
		foreach(j, depth) { cputs("  "); }
		printk("%s\n", dir->entries[i].d.name);
		if (dir->entries[i].d.type == RAMFS_DIR)
			ramfs_dump_dir(&dir->entries[i].d);
		depth--;
	}
}

void ramfs_append_to_dir(char* name, ramfs_dir_entry ent, ramfs_dir* d) {
	d->entries = krealloc(d->entries, (d->num_entries+1)*sizeof(ramfs_dir_entry));

	d->entries[d->num_entries].d.type = ent.d.type;
	if (d->entries[d->num_entries].d.type == RAMFS_DIR) {
		d->entries[d->num_entries].d.entries = ent.d.entries;
		d->entries[d->num_entries].d.num_entries = ent.d.num_entries;
	} else {
		d->entries[d->num_entries].f.content = ent.f.content;
		d->entries[d->num_entries].f.length = ent.f.length;
	}
	strcpy(name, d->entries[d->num_entries].f.name);
	d->num_entries++;
}

void ramfs_mount(partition* fs, char* path) {
	// RAM fájlrendszer létrehozása
	ramfs_header* hdr = kmalloc(sizeof(ramfs_header));

	// Mount listához hozzáadás
	strcpy(path, mnts[num_mnts].path);
	mnts[num_mnts].path_length = strlen(path);
	mnts[num_mnts].p = kmalloc(sizeof(partition));
	*(mnts[num_mnts].p) = (partition) {
		.drive = (void*)hdr,
		.endlba = 0,
		.startlba = 0,
		.type = PART_OWN,
		.fstype = FSTYPE_RAMFS,
		.name = "ramfs",
	};
	num_mnts++;

	// A "gyökérkönyvtár" (ki beszél így?) létrehozása
	ramfs_dir* fsroot = &hdr->root;
	fsroot->name[0] = '\0';
	fsroot->entries = 0;
	fsroot->num_entries = 0;
	fsroot->type = RAMFS_DIR;

	ramfs_mkdir(mnts[0].p, "/mappa");
	ramfs_create(mnts[0].p, "/mappa/file");
	ramfs_create(mnts[0].p, "/mappa/fajl");
	ramfs_mkdir(mnts[0].p, "/mappa/mappa2");
	ramfs_create(mnts[0].p, "/mappa/mappa2/fghjk");

	// char tesztbuf[] = "Hello world!\n";
	// ramfs_write(mnts[0].p, "/mappa/file", tesztbuf, sizeof(tesztbuf));
	// printk("Kiírt: %s", tesztbuf);

	// char tesztbuf2[14];
	// ramfs_read(mnts[0].p, "/mappa/file", tesztbuf2, 14);
	// printk("Visszaolvasott: %s", tesztbuf2);
}

void ramfs_create(partition* fs, char* path) {
	ramfs_dir* d = &((ramfs_header*)(fs->drive))->root;

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') {
			len++;
			path++;
		}

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		if (!*path) {
			ramfs_append_to_dir(
				dirname,
				(ramfs_dir_entry) {
					.f.content = 0,
					.f.length = 0,
					.f.type = RAMFS_FILE,
				},
				d
			);

			break;
		}

		for (u32 i = 0; i < d->num_entries; i++) {
			if (!strcmp(d->entries[i].f.name, dirname)) {
				d = (void*)&d->entries[i];
			}
		}
	}
}

void ramfs_mkdir(partition* fs, char* path) {
	ramfs_dir* d = &((ramfs_header*)(fs->drive))->root;

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') {
			len++;
			path++;
		}

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		if (!*path) {
			ramfs_append_to_dir(
				dirname,
				(ramfs_dir_entry) {
					.d.entries = 0,
					.d.num_entries = 0,
					.d.type = RAMFS_DIR,
				},
				d
			);

			break;
		}

		for (u32 i = 0; i < d->num_entries; i++) {
			if (!strcmp(dirname, d->entries[i].f.name)) {
				d = (void*)&d->entries[i];
			}
		}
	}
}

void ramfs_readdir(partition* fs, char* path, dd** into) {
	ramfs_dir* d = &((ramfs_header*)(fs->drive))->root;

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') {
			len++;
			path++;
		}

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		if (!*path) {
			// dirname: megnyitandó mappa neve
			for (u32 i = 0; i < d->num_entries; i++) {
				if (!strcmp(dirname, d->entries[i].f.name)) {
					d = (void*)&d->entries[i];
					break;
				}
			}
			u32 num_entries = d->num_entries;
			*into = kmalloc(sizeof(dd) + sizeof((*into)->entries[0]) * num_entries);
			for (u32 i = 0; i < num_entries; i++) {
				kfree((*into)->entries[i].name);
				(*into)->entries[i].name = kmalloc(strlen(d->entries[i].f.name));
				strcpy(d->entries[i].f.name, (*into)->entries[i].name);
				(*into)->entries[i].attrs = 0;
			}

			break;
		}

		for (u32 i = 0; i < d->num_entries; i++) {
			if (!strcmp(dirname, d->entries[i].f.name)) {
				d = (void*)&d->entries[i];
				break;
			}
		}
	}
}

void ramfs_read(partition* fs, char* path, void* into, u64 bytes) {
	ramfs_dir* d = &((ramfs_header*)(fs->drive))->root;

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') {
			len++;
			path++;
		}

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		if (!*path) {
			ramfs_file* f = 0;
			for (u32 i = 0; i < d->num_entries; i++) {
				if (!strcmp(dirname, d->entries[i].f.name)) {
					f = (void*)&d->entries[i];
				}
			}
			assert(f);
			assert(f->type == RAMFS_FILE);

			if (bytes > f->length) bytes = f->length;
			memcpy(into, f->content, bytes);

			return;
		}

		for (u32 i = 0; i < d->num_entries; i++) {
			if (!strcmp(dirname, d->entries[i].f.name)) {
				d = (void*)&d->entries[i];
			}
		}
	}
}

u64 ramfs_get_size(partition* fs, char* path) {
	ramfs_dir* d = &((ramfs_header*)(fs->drive))->root;

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') {
			len++;
			path++;
		}

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		if (!*path) {
			ramfs_file* f = 0;
			for (u32 i = 0; i < d->num_entries; i++) {
				if (!strcmp(dirname, d->entries[i].f.name)) {
					f = (void*)&d->entries[i];
				}
			}
			assert(f);
			assert(f->type == RAMFS_FILE);

			return f->length;
		}

		for (u32 i = 0; i < d->num_entries; i++) {
			if (!strcmp(dirname, d->entries[i].f.name)) {
				d = (void*)&d->entries[i];
			}
		}
	}
}

void ramfs_write(partition* fs, char* path, void* from, u64 bytes) {
	ramfs_dir* d = &((ramfs_header*)(fs->drive))->root;

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') {
			len++;
			path++;
		}

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		if (!*path) {
			ramfs_file* f = 0;
			for (u32 i = 0; i < d->num_entries; i++) {
				if (!strcmp(dirname, d->entries[i].f.name)) {
					f = (void*)&d->entries[i];
				}
			}
			assert(f);
			assert(f->type == RAMFS_FILE);

			if (!f->content) {
				f->content = kmalloc(bytes);
				f->length = bytes;
			} else {
				if (f->length < bytes) {
					// TODO
					// kfree(f->content);
					f->content = kmalloc(bytes);
					f->length = bytes;
				}
			}
			memcpy(f->content, from, bytes);
			return;
		}

		for (u32 i = 0; i < d->num_entries; i++) {
			if (!strcmp(dirname, d->entries[i].f.name)) {
				d = (void*)&d->entries[i];
			}
		}
	}
}
