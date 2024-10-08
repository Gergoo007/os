#pragma once

#include <util/types.h>
#include <util/attrs.h>

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
	u8 timer : 1;
	u8 reserved[31];
} dtree_root;

typedef struct _attr_packed dtree_dev {
	dtree_hdr h;
	u64 data[4];
} dtree_dev;

typedef struct _attr_packed dtree_pci_bus {
	dtree_hdr h;
	u8 pcie : 1;
	u8 data[31];
} dtree_pci_bus;

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
