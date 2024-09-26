#pragma once

#include <acpi/acpi.h>

enum {
	PCI_SATA = 0x0106,
	PCI_NVM = 0x0108,
	PCI_USB = 0x0c03,
};

#define PCI_CFG_ADDR 0xcf8
#define PCI_CFG_DATA 0xcfc

typedef union _attr_packed pci_hdr {
	struct _attr_packed {
		u16 vendor;
		u16 product;
		u16 cmd;
		u16 status;
		u8 rev_id;
		u8 prog_if;
		union {
			struct {
				u8 subclass;
				u8 class;
			};
			u16 combclass;
		};
		u8 cache_size;
		u8 latency_timer;
		u8 hdr_type;
		u8 bist;

		union {
			struct {
				u32 bars[6];
				u32 cardbus_cis;
				u16 subsys_vendor;
				u16 subsys_id;
				u32 expansion_rom_addr;
				u8 capabilities;
				u32 : 24;
				u32 : 32;
				u8 int_line;
				u8 int_pin;
				u8 min_grant;
				u8 max_latency;
			} type0;

			struct {
				u32 bars[2];
				u8 primary_bus_num;
				u8 secondary_bus_num;
				u8 subordinate_bus_num;
				u8 secondary_latency_timer;
				u8 io_base;
				u8 io_limit;
				u16 secondary_status;
				u16 mem_base;
				u16 mem_limit;
				u16 prefetch_mem_base;
				u16 prefetch_mem_limit;
				u32 prefetchable_base_upper;
				u32 prefetchable_limit_upper;
				u16 io_base_upper;
				u16 io_limit_upper;
				u8 capabilities;
				u32 : 24;
				u32 expansion_rom_addr;
				u8 int_line;
				u8 int_pin;
				u16 bridge_ctrl;
			} type1;

			struct {
				// TODO
			} type2;
		};
	};
	struct _attr_packed {
		u32 dword[16];
	};
} pci_hdr;

extern mcfg* mc;

void pci_init();
u64 pci_bar_addr(pci_hdr* d, u8 bar);
u8 pci_bar_is_io(pci_hdr* d, u8 bar);
