#pragma once

#include <dtree/tree.h>

typedef struct _attr_packed fat32_bpb {
	u8 jmp[3];
	char oem[8];
	u16 bytes_per_sector;
	u8 sectors_per_cluster;
	u16 reserved_sectors;
	u8 num_fats;
	u16 num_root_dir_entries;
	u16 num_sectors;
	u8 media_desc_type;
	u16 num_sectors_per_fat;
	u16 num_sectors_per_track;
	u16 num_heads;
	u32 num_hidden_sectors;
	u32 large_num_sectors;

	// Extended BPB
	union _attr_packed {
		struct _attr_packed {
			u8 drive_number;
			u8 windows_flags;
			u8 sign;
			u32 volumeid_serial;
			char label[11];
			char system[8];
			u8 boot_code[448];
			u16 boot_signature; // 0xaa55
		} ebpb;
		struct _attr_packed {
			u32 sectors_per_fat;
			u16 flags;
			u16 fat_ver;
			u32 root_dir_cluster_number;
			u16 fsinfo_lba;
			u16 bootsect_bak_lba;
			u8 r0[12];
			u8 drive_num;
			u8 windows_flags;
			u8 sign;
			u32 volumeid_serial;
			char label[11];
			char system[8];
			u8 boot_code[420];
			u16 boot_signature; // 0xaa55
		} ebpb_fat32;
	};
} fat32_bpb;

typedef struct _attr_packed fat32_fsinfo {
	u32 sign; // 0x41615252
	u8 r0[480];
	u32 sign2; // 0x61417272
	u32 last_known_num_free_clusters;
	u32 free_cluster_begin;
	u8 r1[12];
	u32 tailsign; // 0xaa550000
} fat32_fsinfo;

typedef struct _attr_packed fat32_std_83 {
	char sfn[11];
	u8 attrs;
	u8 winnt;
	struct _attr_packed {
		u8 milliseconds;
		u8 seconds : 5;
		u8 minute : 6;
		u8 hour : 5;
		u8 day : 5;
		u8 month : 4;
		u8 year : 7;
	} creation;

	struct _attr_packed {
		u8 day : 5;
		u8 month : 4;
		u8 year : 7;
	} last_access;

	u16 first_cluster_higher;
	struct _attr_packed {
		u8 hour : 5;
		u8 minute : 6;
		u8 seconds : 5;
		u8 year : 7;
		u8 month : 4;
		u8 day : 5;
	} last_mod;
	u16 first_cluster_lower;
	u32 filesize;
} fat32_std_83;

typedef struct _attr_packed fat32_lfn {
	struct _attr_packed {
		u8 number : 5;
		u8 : 1;
		u8 last_lfn_entry : 1;
	} order;
	wchar n0[5];
	u8 attr;
	u8 entry_type;
	u8 sfn_checksum;
	wchar n1[6];
	u16 : 16;
	wchar n2[2];
} fat32_lfn;

typedef union _attr_packed {
	fat32_std_83 std;
	fat32_lfn lfn;
} fat32_entry;

enum {
	FAT_ATTR_RO = 1,
	FAT_ATTR_HIDDEN = 1 << 1,
	FAT_ATTR_SYS = 1 << 2,
	FAT_ATTR_VOL_ID = 1 << 3,
	FAT_ATTR_DIR = 1 << 4,
	FAT_ATTR_ARCHIVE = 1 << 5,
	FAT_ATTR_LFN = FAT_ATTR_RO | FAT_ATTR_HIDDEN | FAT_ATTR_SYS | FAT_ATTR_VOL_ID,
};

void fat32_register_module();
void fat32_mount(partition* part, char* path);
void fat32_read(partition* fs, char* path, void* into, u64 bytes);
u64 fat32_get_size(partition* fs, char* path);
void fat32_write(partition* fs, char* path, void* from, u64 bytes);
void fat32_create(partition* fs, char* path);
void fat32_mkdir(partition* fs, char* path);
