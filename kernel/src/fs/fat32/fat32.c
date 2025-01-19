#include <config.h>
#include <fs/fat32/fat32.h>
#include <mm/vmm.h>
#include <util/mem.h>
#include <util/string.h>
#include <gfx/console.h>
#include <fs/vfs/vfs.h>

typedef struct fat32_dir {
	fat32_entry* entries;
	u32 num_entry;
} fat32_dir;

void fat32_register_module() {
	fs_vtables[FSTYPE_FAT32] = (fs_vtable) {
		.fs_mount = fat32_mount,
		.fs_read = fat32_read,
		.fs_write = fat32_write,
		.fs_create = fat32_create,
		.fs_mkdir = fat32_mkdir,
		.fs_get_size = fat32_get_size,
		.fs_readdir = fat32_readdir,
	};
}

// Egyelőre csak FAT32 support
void fat32_mount(partition* part, char* path) {
	// fat32_bpb* bpb = kmalloc(sizeof(fat32_bpb));
	// drive_read(part->drive, part->startlba, sizeof(fat32_bpb), bpb);

	fat32_bpb* bpb = vmm_alloc();
	drive_read(part->drive, part->startlba, 4096, bpb);

	// TODO: kfree() -> umount
	part->f32c = kmalloc(sizeof(fat32_cache));
	part->f32c->bpb = bpb;
	part->f32c->lba_fats = part->startlba + bpb->reserved_sectors;
	part->f32c->lba_root_dir = part->f32c->lba_fats + bpb->num_fats * bpb->ebpb_fat32.sectors_per_fat;
	part->f32c->lba_data = part->f32c->lba_root_dir + ((bpb->num_root_dir_entries * 32 + bpb->bytes_per_sector - 1) / bpb->bytes_per_sector);
	part->f32c->fat = kmalloc(bpb->ebpb_fat32.sectors_per_fat * 512);
	drive_read(part->drive, part->f32c->lba_fats, bpb->ebpb_fat32.sectors_per_fat * 512, part->f32c->fat);

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
		.cache = part->cache,
	};
	num_mnts++;
}

u128 fat32_read_entry(partition* fs, char* path) {
	// Első 64 bit: memóriacím
	// Utána levő 63 bit: fájlok száma/fájlméret
	// Utolsó 1 bit: 1 ha mappa; 0 ha fájl
	u128 ret = 0;

	fat32_bpb* bpb = fs->f32c->bpb;

	fat32_entry* d = vmm_alloc();
	drive_read(fs->drive, fs->f32c->lba_root_dir, 512, d);

	wchar* wname = kmalloc(sizeof(wchar) * 255);
	memset(wname, 0, sizeof(wchar) * 255);

	char* name = kmalloc(sizeof(char) * 255);
	memset(name, 0, sizeof(char) * 255);

	char* search = path;

	while (1) {
		while (*search == '/') search++;
		u32 namelen = 0;
		while (search[namelen] != '/' && search[namelen] != '\0') namelen++;

		for (u32 i = 0; d[i].lfn.attr; i++) {
			if (d[i].lfn.attr == FAT_ATTR_LFN) {
				// Filenév
				memcpy(wname+((d[i].lfn.order.number-1)*13), d[i].lfn.n0, 10);
				memcpy(wname+((d[i].lfn.order.number-1)*13)+5, d[i].lfn.n1, 12);
				memcpy(wname+((d[i].lfn.order.number-1)*13)+11, d[i].lfn.n2, 4);
			} else if (d[i].lfn.attr & FAT_ATTR_VOL_ID) {

			} else {
				utf16_to_ascii(wname, name);
				memset(wname, 0, sizeof(wchar) * 255);

				if (!*search) {
					error("Nincs ilyen file: %s", path);
					while (1);
				}

				// TODO
				if (!strncmp(search, name, namelen) && !name[namelen]) {
					if (d[i].std.attrs & FAT_ATTR_DIR) {
						// Mappa
						const u32 unit = bpb->sectors_per_cluster * 512;
						u64 size = 1;
						u32 cluster = d[i].std.first_cluster_lower | (d[i].std.first_cluster_higher << 16);
						while ((fs->f32c->fat[cluster] & 0x0fffffff) != 0x0fffffff) size++;
						printk("size: %d\n", size*512);

						if (search[namelen]) {
							if (size*unit > 512)
								error("Mappa túl nagy, hiányozhatnak fájlok!");
							drive_read(fs->drive, fs->f32c->lba_data + (d[i].std.first_cluster_lower-2)*bpb->sectors_per_cluster, 4096, d);
						} else {
							ret |= (u64)kmalloc(size*unit);
							ret |= (u128)1 << 127;
							ret |= (u128)(size*unit / sizeof(fat32_entry)) << 64;

							drive_read(fs->drive, fs->f32c->lba_data + (d[i].std.first_cluster_lower-2)*bpb->sectors_per_cluster, size*unit, (void*)ret);

							goto exit;
						}
					} else {
						// Fájl
						ret |= (u64)kmalloc(sizeof(fat32_entry));
						memcpy((void*)ret, &d[i], sizeof(fat32_entry));
						goto exit;
					}

					goto next;
				}
			}
		}
next:

		search += namelen;
	}

exit:
	kfree(d);
	kfree(wname);
	kfree(name);

	return ret;
}

void fat32_read(partition* fs, char* path, void* into, u64 bytes) {
	struct { fat32_entry* addr; u64 sz : 63; u8 dir : 1; } asd;
	*(u128*)&asd = fat32_read_entry(fs, path);
	if (asd.dir) {
		errno = EISDIR;
		error("Not a file");
		return;
	}

	const u32 unit = fs->f32c->bpb->sectors_per_cluster;
	for (u32 i = asd.addr->std.first_cluster_lower | (asd.addr->std.first_cluster_higher << 16);; i++) {
		drive_read(fs->drive, fs->f32c->lba_data + (i-2) * unit, 512, into);
		into += unit * fs->f32c->bpb->bytes_per_sector;

		if ((fs->f32c->fat[i] & 0x0fffffff) >= 0x0ffffff8) break;
	}
}

void fat32_readdir(partition* fs, char* path, dd** into) {
	struct { fat32_entry* addr; u64 sz : 63; u8 dir : 1; } asd;
	*(u128*)&asd = fat32_read_entry(fs, path);
	if (!asd.dir) {
		errno = ENOTDIR;
		return;
	} else {
		wchar* wname = kmalloc(sizeof(wchar) * 255);
		char* name = kmalloc(sizeof(char) * 255);

		u32 num_files = 0;
		for (u32 i = 0; asd.addr[i].lfn.attr && i < asd.sz; i++) {
			if (!(asd.addr[i].lfn.attr & FAT_ATTR_LFN) && !(asd.addr[i].lfn.attr & FAT_ATTR_VOL_ID)) {
				num_files++;
			}
		}

		dd* p = kmalloc(sizeof(*p) + sizeof(p->entries[0]) * num_files);
		p->num_entries = num_files;
		
		*into = p;

		u32 entry = 0;
		for (u32 i = 0; asd.addr[i].lfn.attr; i++) {
			if (asd.addr[i].lfn.attr & FAT_ATTR_LFN) {
				// Filenév
				memcpy(wname+((asd.addr[i].lfn.order.number-1)*13), asd.addr[i].lfn.n0, 10);
				memcpy(wname+((asd.addr[i].lfn.order.number-1)*13)+5, asd.addr[i].lfn.n1, 12);
				memcpy(wname+((asd.addr[i].lfn.order.number-1)*13)+11, asd.addr[i].lfn.n2, 4);
			} else if (asd.addr[i].lfn.attr & FAT_ATTR_VOL_ID) {

			} else {
				utf16_to_ascii(wname, name);
				memset(wname, 0, sizeof(wchar) * 255);
				if (!*name) {
					// Ha nincs LFN név, használjuk a 8.3-asat
					// strcpy(asd.addr[i].std.name, name);
					u32 j = 0;
					while (asd.addr[i].std.name[j] != ' ') {
						name[j] = asd.addr[i].std.name[j];
						j++;
					}
				}

				p->entries[entry].name = kmalloc(strlen(name));
				strcpy(name, p->entries[entry].name);

				memset(name, 0, sizeof(char) * 255);
				entry++;
			}
		}

		kfree(name);
		kfree(wname);
	}
}

u64 fat32_get_size(partition* fs, char* path) {
	struct { fat32_entry* addr; u64 sz : 63; u8 dir : 1; } asd;
	*(u128*)&asd = fat32_read_entry(fs, path);
	if (asd.dir) {
		errno = EISDIR;
		error("Not a file");
		return 0;
	} else {
		return asd.addr->std.filesize;
	}
}

void fat32_write(partition* fs, char* path, void* from, u64 bytes) {

}

void fat32_create(partition* fs, char* path) {

}

void fat32_mkdir(partition* fs, char* path) {

}
