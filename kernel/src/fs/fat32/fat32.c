#include <fs/fat32/fat32.h>
#include <mm/vmm.h>
#include <util/mem.h>
#include <util/string.h>
#include <gfx/console.h>
#include <fs/vfs/vfs.h>

// fs_vtable fat32_vt = {
// 	.fs_init = fat32_init,
// 	.fs_readf = fat32_readf,
// };

void fat32_register_module() {
	fs_vtables[FSTYPE_FAT32] = (fs_vtable) {
		.fs_mount = fat32_mount,
		.fs_read = fat32_read,
		.fs_write = fat32_write,
		.fs_create = fat32_create,
		.fs_mkdir = fat32_mkdir,
		.fs_get_size = fat32_get_size,
	};
}

// Egyelőre csak FAT32 support
void fat32_mount(partition* part, char* path) {
	fat32_bpb* bpb = vmm_alloc();
	drive_read(part->drive, part->startlba, 4096, bpb);

	// // Offszetek
	// u64 lba_fats = part->startlba + bpb->reserved_sectors;
	// // Ha FAT12/16-ra portolnék: bpb->num_sectors_per_fat
	// u64 lba_root_dir = lba_fats + bpb->num_fats * bpb->ebpb_fat32.sectors_per_fat; // bpb->num_sectors_per_fat;
	// u64 lba_data = lba_root_dir + ((bpb->num_root_dir_entries * 32 + bpb->bytes_per_sector - 1) / bpb->bytes_per_sector);

	// fat32_entry* entries = vmm_alloc();
	// u32 num_entries = 0;
	// u32 longest_lfn_chain = 1;
	// drive_read(part->drive, lba_root_dir, 1, entries);
	// for (u32 i = 0; entries[i].lfn.attr; i++) {
	// 	if (entries[i].lfn.attr == FAT_ATTR_LFN) {
	// 		if (i && entries[i-1].lfn.attr == FAT_ATTR_LFN) longest_lfn_chain++;
	// 	} else
	// 		num_entries++;
	// }

	// wchar* wname = kmalloc(sizeof(wchar) * longest_lfn_chain * 13);
	// memset(wname, 0, sizeof(wchar) * longest_lfn_chain * 13);
	// for (u32 i = 0; entries[i].lfn.attr; i++) {
	// 	if (entries[i].lfn.attr == FAT_ATTR_LFN) {
	// 		memcpy(wname+((entries[i].lfn.order.number-1)*13), entries[i].lfn.n0, 10);
	// 		memcpy(wname+((entries[i].lfn.order.number-1)*13)+5, entries[i].lfn.n1, 12);
	// 		memcpy(wname+((entries[i].lfn.order.number-1)*13)+11, entries[i].lfn.n2, 4);
	// 	} else if (entries[i].lfn.attr & FAT_ATTR_DIR) {
	// 		char* name = kmalloc(wstrlen(wname)*2);
	// 		utf16_to_ascii(wname, name);
	// 		printk("Mappa: %s\n", name);

	// 		// vfs_add_dir(fs_root, name, 1);

	// 		memset(wname, 0, sizeof(wchar) * longest_lfn_chain * 13);
	// 	} else if (entries[i].lfn.attr & FAT_ATTR_VOL_ID) {
	// 		// TODO: lehet hogy nem LFN
	// 		char* name = kmalloc(wstrlen(wname)*2);
	// 		utf16_to_ascii(wname, name);
	// 		printk("Volume ID: %s\n", name);
		
	// 		memset(wname, 0, sizeof(wchar) * longest_lfn_chain * 13);
	// 	} else {
	// 		char* name = kmalloc(wstrlen(wname)*2);
	// 		utf16_to_ascii(wname, name);
	// 		printk("Fájl: %s\n", name);

	// 		// vfs_add_file(fs_root, name, entries[i].std.filesize);

	// 		memset(wname, 0, sizeof(wchar) * longest_lfn_chain * 13);
	// 	}
	// }

	// Mount listához hozzáadás
	strcpy(path, mnts[num_mnts].path);
	mnts[num_mnts].path_length = strlen(path);
	mnts[num_mnts].p = kmalloc(sizeof(partition));
	*(mnts[num_mnts].p) = (partition) {
		.drive = part->drive,
		.startlba = part->startlba,
		.endlba = part->endlba,
		.type = PART_ESP,
		.fstype = FSTYPE_FAT32,
		.name = "fat",
	};
	num_mnts++;
}

void fat32_read(partition* fs, char* path, void* into, u64 bytes) {
	fat32_bpb* bpb = vmm_alloc();
	drive_read(fs->drive, fs->startlba, 4096, bpb);

	// Offszetek
	u64 lba_fats = fs->startlba + bpb->reserved_sectors;
	// Ha FAT12/16-ra portolnék: bpb->num_sectors_per_fat
	u64 lba_root_dir = lba_fats + bpb->num_fats * bpb->ebpb_fat32.sectors_per_fat; // bpb->num_sectors_per_fat;
	u64 lba_data = lba_root_dir + ((bpb->num_root_dir_entries * 32 + bpb->bytes_per_sector - 1) / bpb->bytes_per_sector);

	fat32_entry* d = kmalloc(sizeof(fat32_entry));
	drive_read(fs->drive, lba_root_dir, sizeof(fat32_entry), d);

	wchar* wname = kmalloc(sizeof(wchar) * 255);
	memset(wname, 0, sizeof(wchar) * 255);

	u32* fat = vmm_alloc();
	drive_read(fs->drive, lba_fats, 4096, fat);

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') { len++; path++; }

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		bool found = false;
		for (u32 i = 0; d[i].lfn.attr; i++) {
			if (d[i].lfn.attr == FAT_ATTR_LFN) {
				memcpy(wname+((d[i].lfn.order.number-1)*13), d[i].lfn.n0, 10);
				memcpy(wname+((d[i].lfn.order.number-1)*13)+5, d[i].lfn.n1, 12);
				memcpy(wname+((d[i].lfn.order.number-1)*13)+11, d[i].lfn.n2, 4);
			} else if (d[i].lfn.attr & FAT_ATTR_DIR) {
				char* name = kmalloc(wstrlen(wname)*2);
				utf16_to_ascii(wname, name);

				if (*path) {
					if (!strcmp(name, dirname)) {
						found = true;
						drive_read(fs->drive, lba_data + d[i].std.first_cluster_lower * bpb->sectors_per_cluster, sizeof(fat32_entry), d);
					}
				}

				memset(wname, 0, sizeof(wchar) * 255);
			} else if (d[i].lfn.attr & FAT_ATTR_VOL_ID) {
				// TODO: lehet hogy nem LFN
				char* name = kmalloc(wstrlen(wname)*2);
				utf16_to_ascii(wname, name);
				printk("Volume ID: %s\n", name);

				memset(wname, 0, sizeof(wchar) * 255);
			} else {
				char* name = kmalloc(wstrlen(wname)*2);
				utf16_to_ascii(wname, name);

				if (!*path) {
					// Nem maradt a path-ből, dirname tartalmazza a fájlnevet
					if (!strcmp(name, dirname)) {
						found = true;
						printk("file martch: %d byte\n", d[i].std.filesize);
						printk("1 cluster is %d bytes\n", bpb->sectors_per_cluster*bpb->bytes_per_sector);
						// Első cluster olvasása
						drive_read(fs->drive, lba_data + (d[i].std.first_cluster_lower-2) * bpb->sectors_per_cluster, bpb->sectors_per_cluster * 512, into);
						into += bpb->sectors_per_cluster * bpb->bytes_per_sector;

						for (u32 j = d[i].std.first_cluster_lower; fat[j] < 0x0FFFFFF8; j++) {
							drive_read(fs->drive, lba_data + (fat[j]-2) * bpb->sectors_per_cluster, bpb->sectors_per_cluster * 512, into);
							into += bpb->sectors_per_cluster * bpb->bytes_per_sector;
						}

						return;
					}
				}

				memset(wname, 0, sizeof(wchar) * 255);
			}
		}
		if (!found) {
			error("nincs ilyen fájl/mappa");
			while (1);
		}
	}
}

u64 fat32_get_size(partition* fs, char* path) {
	fat32_bpb* bpb = vmm_alloc();
	drive_read(fs->drive, fs->startlba, sizeof(fat32_bpb), bpb);

	// Offszetek
	u64 lba_fats = fs->startlba + bpb->reserved_sectors;
	// Ha FAT12/16-ra portolnék: bpb->num_sectors_per_fat
	u64 lba_root_dir = lba_fats + bpb->num_fats * bpb->ebpb_fat32.sectors_per_fat; // bpb->num_sectors_per_fat;
	u64 lba_data = lba_root_dir + ((bpb->num_root_dir_entries * 32 + bpb->bytes_per_sector - 1) / bpb->bytes_per_sector);

	fat32_entry* d = vmm_alloc();
	drive_read(fs->drive, lba_root_dir, sizeof(fat32_entry), d);

	wchar* wname = kmalloc(sizeof(wchar) * 255);
	memset(wname, 0, sizeof(wchar) * 255);

	u32* fat = vmm_alloc();
	drive_read(fs->drive, lba_fats, 4096, fat);

	while (1) {
		if (*path == '/') path++;
		u32 len = 0;
		while (*path != '\0' && *path != '/') { len++; path++; }

		char* dirname = kmalloc(len+1);
		memcpy(dirname, path - len, len);
		dirname[len] = 0;

		bool found = false;
		for (u32 i = 0; d[i].lfn.attr; i++) {
			if (d[i].lfn.attr == FAT_ATTR_LFN) {
				memcpy(wname+((d[i].lfn.order.number-1)*13), d[i].lfn.n0, 10);
				memcpy(wname+((d[i].lfn.order.number-1)*13)+5, d[i].lfn.n1, 12);
				memcpy(wname+((d[i].lfn.order.number-1)*13)+11, d[i].lfn.n2, 4);
			} else if (d[i].lfn.attr & FAT_ATTR_DIR) {
				char* name = kmalloc(wstrlen(wname)*2);
				utf16_to_ascii(wname, name);

				if (*path) {
					if (!strcmp(name, dirname)) {
						found = true;
						drive_read(fs->drive, lba_data + d[i].std.first_cluster_lower * bpb->sectors_per_cluster, sizeof(fat32_entry), d);
					}
				}

				memset(wname, 0, sizeof(wchar) * 255);
			} else if (d[i].lfn.attr & FAT_ATTR_VOL_ID) {
				// TODO: lehet hogy nem LFN
				char* name = kmalloc(wstrlen(wname)*2);
				utf16_to_ascii(wname, name);
				printk("Volume ID: %s\n", name);

				memset(wname, 0, sizeof(wchar) * 255);
			} else {
				char* name = kmalloc(wstrlen(wname)*2);
				utf16_to_ascii(wname, name);

				if (!*path) {
					// Nem maradt a path-ből, dirname tartalmazza a fájlnevet
					if (!strcmp(name, dirname)) {
						found = true;
						return d[i].std.filesize;
					}
				}

				memset(wname, 0, sizeof(wchar) * 255);
			}
		}
		if (!found) {
			error("nincs ilyen fájl/mappa");
			while (1);
		}
	}
}

void fat32_write(partition* fs, char* path, void* from, u64 bytes) {

}

void fat32_create(partition* fs, char* path) {

}

void fat32_mkdir(partition* fs, char* path) {

}
