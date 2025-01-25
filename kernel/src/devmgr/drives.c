#include <devmgr/drives.h>
#include <devmgr/devmgr.h>
#include <mm/vmm.h>
#include <util/mem.h>
#include <fs/gpt.h>
#include <gfx/console.h>

enum PART_TYPES guid_to_enum(u8* guid) {
	u32 part0 = *(u32*)(guid);
	u16 part1 = *(u16*)(guid+4);
	u16 part2 = *(u16*)(guid+6);
	u8* remaining = guid+8;

	u8 failed = 0;

	CHECK_GUID(part0, part1, part2, remaining);

	return PART_UNKNOWN;
}

void drive_init(dev_drive* d) {
	gpt_hdr* h = vmm_alloc();
	drive_read(d, 1, sizeof(gpt_hdr), h);

	part_table* tbl = kmalloc(sizeof(part_table));
	tbl->num_parts = 0;
	FOREACH_PART(h, i) {
		if (!*(u64*)h->entries[i].part_type) continue;

		u32 type = guid_to_enum(h->entries[i].part_type);
		tbl->parts[tbl->num_parts] = (typeof(tbl->parts[tbl->num_parts])) {
			.type = type,
			.startlba = h->entries[i].start_lba,
			.endlba = h->entries[i].end_lba,
			.drive = d,
		};
		memcpy(tbl->parts[tbl->num_parts].name, h->entries[i].name, 72);

		tbl->num_parts++;
	}
	d->tbl = tbl;

	vmm_free(h);
}

void drive_read(dev_drive* d, u64 start, u64 bytes, void* into) {
	switch (d->hdr.type) {
		// case DEV_ATA: {
		// 	// if (d->identity->ExtendedNumberOfUserAddressableSectors) {
		// 	// 	if (bytes > d->identity->ExtendedNumberOfUserAddressableSectors*512)
		// 	// 		bytes = d->identity->ExtendedNumberOfUserAddressableSectors*512;
		// 	// } else if (d->identity->UserAddressableSectors) {
		// 	// 	if (bytes > d->identity->UserAddressableSectors*512)
		// 	// 		bytes = d->identity->UserAddressableSectors*512;
		// 	// }
		// 	// if (bytes < 512) {
		// 	// 	// Bounce buffer
		// 	// 	void* tmp = kmalloc(512);
		// 	// 	ahci_read((dtree_pci_dev*)parent, d->port, start, 512, tmp);
		// 	// 	memcpy(into, tmp, bytes);
		// 	// 	kfree(tmp);
		// 	// } else {
		// 	// 	ahci_read((dtree_pci_dev*)parent, d->port, start, bytes, into);
		// 	// }
		// 	// break;
		// }
		default: {
			error("Nem támogatott meghajtó típus: %s\n", dev_types);
		}
	}
}
