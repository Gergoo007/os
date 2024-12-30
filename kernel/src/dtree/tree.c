#include <dtree/tree.h>

#include <mm/vmm.h>
#include <util/mem.h>
#include <gfx/console.h>
#include <storage/ahci/ahci.h>
#include <usb/hci/uhci.h>
#include <usb/hci/ehci.h>

dtree_dev* dtree;
static bitmap devs;

const char* dtree_types[] = {
	TYPE_LIST
};

void dtree_init() {
	dtree = kmalloc(MAX_NUM_DEVS * sizeof(dtree_dev));
	memset(dtree, 0, MAX_NUM_DEVS * sizeof(dtree_dev));
	// Align
	if ((u64)dtree & 0b111111) dtree = (void*)(((u64)dtree | 0b111111) + 1);

	devs.base = kmalloc(MAX_NUM_DEVS / 8);
	bm_init(&devs);

	// Index 0: a számítógép
	bm_set(&devs, 0, 1);
	dtree[0].h = (dtree_hdr) {
		.type = DEV_ROOT,
	};

	// Index 1: A PCI(e) busz
	bm_set(&devs, 1, 1);
	dtree[1].h = (dtree_hdr) {
		.parent = 0,
	};
	dtree[0].h.children[0] = 1;
	dtree[0].h.num_children = 1;
}

// Ha hozzá van adva minden a device treehez,
// le lehet ezt szépen futtatni
void dtree_walk() {
	for (u32 i = 0; i < dtree[1].h.num_children; i++) {
		dtree_pci_dev* d = (dtree_pci_dev*)&dtree[dtree[1].h.children[i]];
		switch (d->h.type) {
			case DEV_PCI_AHCI: {
				ahci_init(d);
				break;
			}

			case DEV_UHCI: {
				// uhci_init(d);
				break;
			}
		}
	}
}

u16 dtree_add_dev(dtree_dev* dev) {
	u16 ind = bm_alloc(&devs);
	dtree[ind] = *dev;
	dtree[ind].h.index = ind;

	if (dtree[dev->h.parent].h.num_children == 32) {
		error("Nem maradt hely az eszköz hozzáadására");
		return 0xffff;
	}

	dtree[dev->h.parent].h.children[dtree[dev->h.parent].h.num_children] = ind;
	dtree[dev->h.parent].h.num_children++;

	return ind;
}

u16 dtree_add_pci_dev(dtree_pci_dev* dev) {
	dev->h.parent = 1;
	return dtree_add_dev((dtree_dev*)dev);
}

u16 dtree_add_usb_dev(dtree_usb_dev* dev) {
	return dtree_add_dev((dtree_dev*)dev);
}

u16 dtree_add_drive(dtree_drive* dev) {
	u16 ret = dtree_add_dev((dtree_dev*)dev);
	drive_init((void*)&dtree[ret]);
	return ret;
}

u16 dtree_add_chipset_dev(dtree_dev* dev) {
	return dtree_add_dev((dtree_dev*)dev);
}
