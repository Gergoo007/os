#pragma once

#include <fs/vfs/vfs.h>

enum {
	RAMFS_DIR,
	RAMFS_FILE,
};

typedef union ramfs_dir_entry ramfs_dir_entry;
typedef struct _attr_packed ramfs_dir {
	char name[VFS_LIMIT_FILENAME];
	ramfs_dir_entry* entries;
	u64 num_entries : 56;
	u8 type;
} ramfs_dir;

typedef struct _attr_packed ramfs_file {
	char name[VFS_LIMIT_FILENAME];
	void* content;
	u64 length : 56;
	u8 type;
} ramfs_file;

typedef union _attr_packed ramfs_dir_entry {
	ramfs_file f;
	ramfs_dir d;
} ramfs_dir_entry;

typedef struct _attr_packed ramfs_header {
	ramfs_dir root;
} ramfs_header;

extern fs_vtable ramfs_vt;

void ramfs_register_module();
void ramfs_mount(partition* fs, char* path);
void ramfs_create(partition* fs, char* path);
void ramfs_mkdir(partition* fs, char* path);
void ramfs_read(partition* fs, char* path, void* into, u64 bytes);
void ramfs_write(partition* fs, char* path, void* from, u64 bytes);

void ramfs_dump_dir(ramfs_dir* dir);
