#pragma once

#include <util/types.h>
#include <util/attrs.h>

#define TYPE_LIST \
	TYPE_ADD(DEV_ROOT, 0) \
	TYPE_ADD(DEV_CPU, 1) \
	TYPE_ADD(DEV_RAM, 2) \
	TYPE_ADD(DEV_CHIPSET, 3) \
	TYPE_ADD(DEV_PCI_BUS, 4) \
	TYPE_ADD(DEV_PCIE_BUS, 5) \
	TYPE_ADD(DEV_PCI_MISC, 6) \
	TYPE_ADD(DEV_PCI_GPU, 7) \
	TYPE_ADD(DEV_PCI_AHCI, 8) \
	TYPE_ADD(DEV_UHCI, 9) \
	TYPE_ADD(DEV_OHCI, 10) \
	TYPE_ADD(DEV_EHCI, 11) \
	TYPE_ADD(DEV_XHCI, 12) \
	TYPE_ADD(DEV_USB_HID, 13) \
	TYPE_ADD(DEV_USB_KBD, 14) \
	TYPE_ADD(DEV_USB_MOU, 15) \
	TYPE_ADD(DEV_USB_MSD, 16) \
	TYPE_ADD(DEV_USB_HUB, 17) \
	TYPE_ADD(DEV_USB_PTP, 18) \
	TYPE_ADD(DEV_USB_MTP, 19) \
	TYPE_ADD(DEV_USB_MISC, 20) \
	TYPE_ADD(DEV_ATA, 21) \
	TYPE_ADD(DEV_ATAPI, 22) \

// enum verzió
#define TYPE_ADD(name, val) name = val,

enum {
	TYPE_LIST
};

// array verzió
#undef TYPE_ADD
#define TYPE_ADD(name, val) [val] = #name,

typedef struct _attr_packed dtree_hdr {
	u16 type;
	u16 parent;
	u16 num_children;
	u16 children[32];
	u16 index;
	// u64 data[4];
} dtree_hdr;

typedef struct _attr_packed dtree_root {
	dtree_hdr h;
	u64 reserved[4];
} dtree_root;

typedef struct _attr_packed dtree_dev {
	dtree_hdr h;
	u64 data[4];
} dtree_dev;

typedef struct _attr_packed dtree_pci_dev {
	dtree_hdr h;
	u16 vendor;
	u16 product;
	u8 cfg_space;
	u8 bus;
	u8 dev;
	u8 fun;
	void* handle;
	u64 reserved[2];
} dtree_pci_dev;

typedef struct _attr_packed dtree_usb_dev {
	dtree_hdr h;
	char* vendor;
	char* product;
	void* hci_handle;
	u8 hci_type;
	u8 speed;
	u8 mps;
	u8 reserved[5];
} dtree_usb_dev;

extern dtree_dev* dtree;

#define dtree_get_child_of_id(id, ind) dtree[dtree[id].h.children[ind]]
#define dtree_get_child(strct, ind) dtree[strct->h.children[ind]]

extern const char* dtree_types[];

void dtree_init();
u16 dtree_add_dev(dtree_dev* dev);
u16 dtree_add_pci_dev(dtree_pci_dev* dev);
u16 dtree_add_usb_dev(dtree_usb_dev* dev);
void dtree_walk();
