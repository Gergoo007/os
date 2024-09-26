#pragma once

#include <pci/pci.h>
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

enum {
	UPID_SETUP = 0b00101101,
	UPID_IN = 0b01101001,
	UPID_OUT = 0b11100001,
};

typedef union _attr_packed ucmd {
	struct _attr_packed {
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

typedef union _attr_packed usts {
	struct _attr_packed {
		u8 intr : 1;
		u8 errintr : 1;
		u8 resume : 1;
		u8 host_sys_err : 1;
		u8 hc_proc_err : 1;
		u8 hchalted : 1;
	};
	u16 usts;
} usts;

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

typedef struct _attr_packed uhci_td {
	// TD LINK PTR
	union _attr_packed {
		struct _attr_packed {
			u8 t : 1;
			u8 q : 1;
			u8 depth : 1;
		};
		u32 link;
	} link;

	// TD CTL & STS
	struct _attr_packed {
		u16 actual_len : 11; // HC által írt
		u8 : 5;
		union _attr_packed {
			u8 sts;
			struct _attr_packed {
				u8 : 7;
				u8 active : 1;
			};
		};
		u8 ioc : 1;
		u8 isoch : 1;
		u8 ls : 1;
		u8 err_counter : 2;
		u8 spd : 1;
		u8 : 2;
	};

	// TD TOKEN
	struct _attr_packed {
		u8 pid;
		u8 addr : 7;
		u8 endp : 4;
		u8 data : 1;
		u8 : 1;
		u16 maxlen : 11;
	};

	// TD BUF PTR
	u32 buffer;

	u32 free[4];
} uhci_td;

typedef struct _attr_packed uhci_qh {
	union _attr_packed {
		struct _attr_packed {
			u8 t : 1;
			u8 q : 1;
		};
		u32 link;
	} head;

	volatile union _attr_packed {
		volatile struct _attr_packed {
			volatile u8 t : 1;
			volatile u8 q : 1;
		};
		volatile u32 link;
	} elem;
} uhci_qh;

typedef struct uhci {
	u16 io;
	uhci_td* tdlist;
	uhci_qh* qhlist;
} uhci;

void uhci_init(pci_hdr* d);
