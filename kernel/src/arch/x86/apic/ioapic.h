#pragma once

#include <pci/pci.h>
#include <util/attrs.h>
#include <devmgr/devmgr.h>

enum {
	IOAPIC_IDENT = 0x00,
	IOAPIC_VER = 0x01,
	IOAPIC_ARB = 0x02,
};

typedef union _attr_packed {
	struct _attr_packed {
		union _attr_packed {
			struct _attr_packed {
				u32 vector : 8;
				u32 delivery : 3;
				u32 destination_logical : 1;
				u32 deliv_sts : 1;
				u32 active_low : 1;
				u32 remote_irr : 1;
				u32 lvl_trig : 1;
				u32 mask : 1;
				u32 : 15;
			};
			u32 lower;
		};

		union _attr_packed {
			struct _attr_packed {
				u32 : 24;
				u8 destination;
			};
			u32 higher;
		};
	};
	u64 raw;
} ioapic_entry;

typedef struct _attr_packed dev_ioapic {
	dev_hdr hdr;
	u32 gsi_base;
	u32 id;
} dev_ioapic;

typedef struct irq_redirect {
	u32 gsi : 30;
	u32 active_low : 1;
	u32 lvl_trig : 1;
} irq_redirect;
extern irq_redirect ioapic_irqs[];

void ioapic_write_entry(irq_redirect gsi, ioapic_entry entry);
ioapic_entry ioapic_get_entry(u32 gsi);
void ioapic_set_mask(u32 gsi, bool mask);
void ioapic_set_vector(u32 gsi, u8 v);
void ioapic_set_destination(u32 gsi, u8 logical, u8 dest);
