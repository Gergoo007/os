#pragma once

#include <util/types.h>
#include <util/attrs.h>
#include <dtree/tree.h>

#define VFS_LIMIT_FILENAME 256

enum {
	FSTYPE_RAMFS,
	FSTYPE_FAT32,
};

typedef struct fd {
	partition* fs;
	struct fs_vtable* vt;
	char relative_path[254]; // a filesystem root-hoz relatív
	u16 attrs;
	u32 num_entries;
	struct {
		char* name;
		u16 attrs;
	} entries[0];
} fd;

typedef struct dd {
	partition* fs;
	struct fs_vtable* vt;
	char relative_path[254]; // a filesystem root-hoz relatív
	u16 attrs;
	u32 num_entries;
	struct {
		char* name;
		u16 attrs;
	} entries[0];
} dd;

typedef struct fs_vtable {
	void (*fs_mount)(partition* fs, char* path);
	void (*fs_read)(partition* fs, char* path, void* into, u64 bytes);
	u64 (*fs_get_size)(partition* fs, char* path);
	void (*fs_write)(partition* fs, char* path, void* from, u64 bytes);
	void (*fs_create)(partition* fs, char* path);
	void (*fs_mkdir)(partition* fs, char* path);
	void (*fs_readdir)(partition* fs, char* path, dd** into);
} fs_vtable;

extern fs_vtable* fs_vtables;

typedef struct vfs_mountp {
	char path[254];
	u16 path_length;
	partition* p;
} vfs_mountp;

extern u32 num_mnts;
extern vfs_mountp* mnts;

void vfs_init();
void vfs_list_mnts();

fd* vfs_open(char* path, char* mode);
void vfs_mkdir(char* path);
void vfs_create(fd* f);
void vfs_read(fd* f, void* into, u64 bytes);
dd* vfs_readdir(char* path);
u64 vfs_get_size(fd* f);
void vfs_write(fd* f, void* from, u64 bytes);
void vfs_close(fd* f);
