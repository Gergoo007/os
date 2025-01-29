#pragma once

#include <util/types.h>
#include <util/dynlist.h>

enum subsystem {
	// chipset
	SUBSYS_CPS = 0b00000000,
	SUBSYS_PCI = 0b00010000,
	SUBSYS_USB = 0b00100000,
	SUBSYS_STG = 0b00110000,
};

#define TYPE_LIST \
	TYPE_ADD(DEV_ROOT, 0 | SUBSYS_CPS) \
	TYPE_ADD(DEV_CPU, 1 | SUBSYS_CPS) \
	TYPE_ADD(DEV_RAM, 2 | SUBSYS_CPS) \
	TYPE_ADD(DEV_CHIPSET, 3 | SUBSYS_CPS) \
	TYPE_ADD(DEV_PCI_BUS, 4 | SUBSYS_CPS) \
	TYPE_ADD(DEV_IOAPIC, 5 | SUBSYS_CPS) \
	\
	TYPE_ADD(DEV_PCI_MISC, 0 | SUBSYS_PCI) \
	TYPE_ADD(DEV_PCI_GPU, 1 | SUBSYS_PCI) \
	TYPE_ADD(DEV_PCI_AHCI, 2 | SUBSYS_PCI) \
	TYPE_ADD(DEV_UHCI, 3 | SUBSYS_PCI) \
	TYPE_ADD(DEV_OHCI, 4 | SUBSYS_PCI) \
	TYPE_ADD(DEV_EHCI, 5 | SUBSYS_PCI) \
	TYPE_ADD(DEV_XHCI, 6 | SUBSYS_PCI) \
	\
	TYPE_ADD(DEV_USB_HID, 0 | SUBSYS_USB) \
	TYPE_ADD(DEV_USB_KBD, 1 | SUBSYS_USB) \
	TYPE_ADD(DEV_USB_MOU, 2 | SUBSYS_USB) \
	TYPE_ADD(DEV_USB_MSD, 3 | SUBSYS_USB) \
	TYPE_ADD(DEV_USB_HUB, 4 | SUBSYS_USB) \
	TYPE_ADD(DEV_USB_PTP, 5 | SUBSYS_USB) \
	TYPE_ADD(DEV_USB_MTP, 6 | SUBSYS_USB) \
	TYPE_ADD(DEV_USB_MISC, 7 | SUBSYS_USB) \
	\
	TYPE_ADD(DEV_ATA, 0 | SUBSYS_STG) \
	TYPE_ADD(DEV_ATAPI, 1 | SUBSYS_STG) \
	TYPE_ADD(DEV_NVME, 2 | SUBSYS_STG) \

// enum verzió
#define TYPE_ADD(name, val) name = (val),

enum dev_type {
	TYPE_LIST
};

// array verzió
#undef TYPE_ADD
#define TYPE_ADD(name, val) [(val)] = #name,

typedef struct dev_hdr {
	struct dev_hdr* parent;
	struct dev_hdr** children; // pointer to pointer array
	union {
		void* handle;
		struct dev_misc_pci_data* pci_data;
	};
	u16 num_children;
	enum dev_type type;
	union {
		u64 slot;
		struct {
			u64 cfg_space : 8;
			u64 bus : 8;
			u64 dev : 8;
			u64 fun : 8;
		} pci_addr;
	};
} dev_hdr;

typedef struct dev_drive {
	dev_hdr hdr;
	u64 sectors;
	struct part_table* tbl;
} dev_drive;

typedef struct dev_misc {
	dev_hdr hdr;
} dev_misc;

typedef struct dev_cpu {
	u8 acpi_id;
	u8 apic_id;
	u8 online_capable : 1;
	u8 online : 1;
} dev_cpu;

typedef struct computer_x86 {
	dev_hdr hdr;
	u32 ps2 : 1;
	u32 uhci_ps2_compat : 1;
	u32 acpi : 1;
	u32 pci_mcfg : 1;
	enum {
		TMR_TSC,
		TMR_PIT,
		TMR_APIC,
		TMR_ACPI,
		TMR_HPET,
		TMR_RTC,
	} tmr;
	u32 num_ioapics;
	struct dev_ioapic* ioapics;
	u32 num_cpus;
	dev_cpu* cpus;
} computer_x86;

extern computer_x86* computer;
extern const char* dev_types[];

extern dynlist drives;

enum signal {
	DEV_SIG_CONNECT = 0,
	DEV_SIG_DISCONNECT = 0,
};

enum trigger {
	DEV_TRIG_VENDOR_PRODUCT 			= 0,
	DEV_TRIG_CLASS 						= 1,
	DEV_TRIG_CLASS_SUBCLASS 			= 2,
	DEV_TRIG_CLASS_SUBCLASS_PROGIF 		= 3,

	DEV_TRIG_PCI_VENDOR_PRODUCT 		= DEV_TRIG_VENDOR_PRODUCT 		| SUBSYS_PCI,
	DEV_TRIG_PCI_CLASS 					= DEV_TRIG_CLASS 				| SUBSYS_PCI,
	DEV_TRIG_PCI_CLASS_SUBCLASS 		= DEV_TRIG_CLASS_SUBCLASS 		| SUBSYS_PCI,
	DEV_TRIG_PCI_CLASS_SUBCLASS_PROGIF 	= DEV_TRIG_CLASS_SUBCLASS_PROGIF| SUBSYS_PCI,

	DEV_TRIG_USB_VENDOR_PRODUCT			= DEV_TRIG_VENDOR_PRODUCT 		| SUBSYS_USB,
	DEV_TRIG_USB_CLASS 					= DEV_TRIG_CLASS 				| SUBSYS_USB,
	DEV_TRIG_USB_CLASS_SUBCLASS 		= DEV_TRIG_CLASS_SUBCLASS 		| SUBSYS_USB,
	DEV_TRIG_USB_CLASS_SUBCLASS_PROGIF 	= DEV_TRIG_CLASS_SUBCLASS_PROGIF| SUBSYS_USB,
};

typedef struct dev_cb_filter {
	enum trigger type;
	union {
		struct {
			u16 vendor;
			u16 product;
		} vp;
		struct {
			u8 class;
			u8 subclass;
			u8 progif;
		} class;
	};
} dev_cb_filter;

void devmgr_init();
void dev_add(void* vparent, void* vchild);
void dev_set_callback(enum signal s, dev_cb_filter filter, void (*onconnect)(dev_misc*), void (*ondisconnect)(dev_misc*));
