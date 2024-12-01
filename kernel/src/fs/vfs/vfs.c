#include <fs/vfs/vfs.h>
#include <fs/vfs/ramfs.h>
#include <fs/fat32/fat32.h>
#include <mm/vmm.h>
#include <util/string.h>
#include <util/mem.h>
#include <gfx/console.h>
#include <dtree/drives.h>
#include <config.h>

u32 num_mnts;
vfs_mountp* mnts;

fs_vtable* fs_vtables;

void vfs_init() {
	mnts = kmalloc(0x1000);
	memset(mnts, 0, 0x1000);

	fs_vtables = kmalloc(sizeof(fs_vtable) * 8);

	ramfs_register_module();
	fat32_register_module();

	ramfs_mount(0, "/");
}

void vfs_list_mnts() {
	printk("Mounted:\n");
	for (u32 i = 0; i < num_mnts; i++) {
		assert(strlen(mnts[i].path) == mnts[i].path_length);
		printk("%s: %s (parttype %d fstype %d)\n", mnts[i].path, mnts[i].p->name, mnts[i].p->type, mnts[i].p->fstype);
	}
}

fd* vfs_open(char* path) {
	// A leghosszabb egyező path lesz a helyes mountpoint
	u32 longest_match = 0;
	for (u32 i = 0; i < num_mnts; i++) {
		if (mnts[i].path_length < longest_match) continue;
		if (!strncmp(mnts[i].path, path, mnts[i].path_length))
			if (i > longest_match) longest_match = i;
	}

	fd* f = kmalloc(sizeof(fd));
	f->attrs = 0;
	f->vt = &fs_vtables[mnts[longest_match].p->fstype];
	f->fs = mnts[longest_match].p;
	strcpy(path + mnts[longest_match].path_length, f->relative_path);
	return f;
}

void vfs_mkdir(char* path) {
	// A leghosszabb egyező path lesz a helyes mountpoint
	u32 longest_match = 0;
	for (u32 i = 0; i < num_mnts; i++) {
		if (mnts[i].path_length < longest_match) continue;
		if (!strncmp(mnts[i].path, path, mnts[i].path_length))
			if (i > longest_match) longest_match = i;
	}

	fd* f = kmalloc(sizeof(fd));
	f->attrs = 0;
	f->vt = &fs_vtables[mnts[longest_match].p->fstype];
	f->fs = mnts[longest_match].p;
	strcpy(path + mnts[longest_match].path_length, f->relative_path);

	f->vt->fs_mkdir(f->fs, f->relative_path);
}

void vfs_create(fd* f) {
	f->vt->fs_create(f->fs, f->relative_path);
}

void vfs_read(fd* f, void* into, u64 bytes) {
	f->vt->fs_read(f->fs, f->relative_path, into, bytes);
}

void vfs_write(fd* f, void* from, u64 bytes) {
	f->vt->fs_write(f->fs, f->relative_path, from, bytes);
}

void vfs_close(fd *f) {
	// kfree(f);
}
