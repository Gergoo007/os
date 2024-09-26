#pragma once

#include <pci/pci.h>
#include <util/attrs.h>

enum {
	IOAPIC_IDENT = 0x00,
	IOAPIC_VER = 0x01,
	IOAPIC_ARB = 0x02,
};

typedef struct ioapic {
	u64 base;
	u32 gsi_base;
	u32 id;
} ioapic;

typedef union _attr_packed {
	struct _attr_packed {
		union _attr_packed {
			struct _attr_packed {
				u8 vector;
				u8 delivery : 3;
				u8 destination_logical : 1;
				u8 deliv_sts : 1;
				u8 active_low : 1;
				u8 remote_irr : 1;
				u8 lvl_trig : 1;
				u8 mask : 1;
				u32 : 17;
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

void ioapic_write_entry(u32 gsi, ioapic_entry entry);
ioapic_entry ioapic_get_entry(u32 gsi);
void ioapic_set_mask(u32 gsi, bool mask);
void ioapic_set_vector(u32 gsi, u8 v);
void ioapic_set_destination(u32 gsi, u8 logical, u8 dest);
