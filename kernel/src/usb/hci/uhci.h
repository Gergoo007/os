#pragma once

#include <pcie/pcie.h>
#include <util/types.h>
#include <util/attrs.h>

enum {
	UCMD = 0x00,
	USTS = 0x02,
	UINTR = 0x04,
	UFRNUM = 0x06,
	UFRBASE = 0x08,
	USOFMOD = 0x0c,
	UPORTSC1 = 0x10,
	UPORTSC2 = 0x12,
};

typedef union _attr_packed ucmd {
	struct {
		u8 run : 1;
		u8 hcreset : 1;
		u8 greset : 1;
		u8 global_suspend : 1;
		u8 global_resume : 1;
		u8 swdbg : 1;
		u8 : 1;
		u8 max : 1;
		u8 : 8;
	};
	u16 ucmd;
} ucmd;

typedef union _attr_packed uport {
	struct {
		u8 connected : 1;
		u8 conn_change : 1;
		u8 port_enabled : 1;
		u8 port_en_changed : 1;
		u8 data : 2;
		u8 resume : 1;
		u8 : 1;
		u8 ls : 1;
		u8 reset : 1;
		u8 : 2;
		u8 suspend : 1;
		u8 : 3;
	};
	u16 uport;
} uport;

void uhci_init(pci_hdr* d);
