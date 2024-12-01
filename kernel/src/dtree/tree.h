#pragma once

#include <util/types.h>
#include <util/attrs.h>
#include <dtree/basetypes.h>
#include <dtree/drives.h>

#define MAX_NUM_DEVS 128

//   0:40	-> Alaplapon található eszközök
//  41:140	-> PCI(e) eszközök
// 141:200	-> USB eszközök
// 201:220	-> Egyéb háttértárak
// 221:255	-> Reserved
// 255		-> Nem definiált/misc

#define TYPE_LIST \
	TYPE_ADD(DEV_ROOT, 0) \
	TYPE_ADD(DEV_CPU, 1) \
	TYPE_ADD(DEV_RAM, 2) \
	TYPE_ADD(DEV_CHIPSET, 3) \
	TYPE_ADD(DEV_PCI_BUS, 4) \
	TYPE_ADD(DEV_IOAPIC, 5) \
	TYPE_ADD(DEV_PCI_MISC, 41) \
	TYPE_ADD(DEV_PCI_GPU, 42) \
	TYPE_ADD(DEV_PCI_AHCI, 43) \
	TYPE_ADD(DEV_UHCI, 44) \
	TYPE_ADD(DEV_OHCI, 45) \
	TYPE_ADD(DEV_EHCI, 46) \
	TYPE_ADD(DEV_XHCI, 47) \
	TYPE_ADD(DEV_USB_HID, 141) \
	TYPE_ADD(DEV_USB_KBD, 142) \
	TYPE_ADD(DEV_USB_MOU, 143) \
	TYPE_ADD(DEV_USB_MSD, 144) \
	TYPE_ADD(DEV_USB_HUB, 145) \
	TYPE_ADD(DEV_USB_PTP, 146) \
	TYPE_ADD(DEV_USB_MTP, 147) \
	TYPE_ADD(DEV_USB_MISC, 148) \
	TYPE_ADD(DEV_ATA, 201) \
	TYPE_ADD(DEV_ATAPI, 202) \

// enum verzió
#define TYPE_ADD(name, val) name = val,

enum {
	TYPE_LIST
};

// array verzió
#undef TYPE_ADD
#define TYPE_ADD(name, val) [val] = #name,

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

typedef struct _attr_packed dtree_cpu {
	dtree_hdr h;
	union _attr_packed {
		struct _attr_packed {
			u8 acpi_id;
			u8 apic_id;
			u8 enabled : 1;
			u8 capable : 1;
		};
		u64 reserved[4];
	};
} dtree_cpu;

extern dtree_dev* dtree;

#define dtree_get_child_of_id(id, ind) ((void*)(&dtree[dtree[id].h.children[ind]]))
#define dtree_get_child(strct, ind) ((void*)(&dtree[(strct)->h.children[ind]]))
#define dtree_for(dev, iter) for (u32 (iter) = 0; (iter) < (dev)->h.num_children; (iter)++)

extern const char* dtree_types[];

void dtree_init();
u16 dtree_add_dev(dtree_dev* dev);
u16 dtree_add_pci_dev(dtree_pci_dev* dev);
u16 dtree_add_usb_dev(dtree_usb_dev* dev);
u16 dtree_add_drive(dtree_drive* dev);
u16 dtree_add_chipset_dev(dtree_dev* dev);
void dtree_walk();
