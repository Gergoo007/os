#pragma once

#include <devmgr/devmgr.h>
#include <util/attrs.h>

typedef volatile struct _attr_packed xhci_caps {
	u8 caplength;
	u8 : 8;
	u16 hciversion;
	union _attr_packed {
		u16 hcsparams1;
		volatile struct _attr_packed {
			u8 maxslots;
			u16 maxintrs : 11;
			u32 : 13;
		};
	};
	union _attr_packed {
		u32 hcsparams2;
		volatile struct _attr_packed {
			u8 ist : 3;
			u8 erstmax : 4;
			u32 : 25;
		};
	};
	union _attr_packed {
		u32 hcsparams3;
		volatile struct _attr_packed {
			u8 u1_dev_exit_latency;
			u8 : 8;
			u16 u2_dev_exit_latency;
		};
	};
	union _attr_packed {
		u32 hccparams1;
		volatile struct _attr_packed {
			u8 longptrs : 1;
			u8 bw_negotiation : 1;
			u8 context_size : 1;
			u8 port_pwr_ctl : 1;
			u8 port_indicators : 1;
			u8 light_hc_reset : 1;
			u8 latency_tolerance_messaging : 1;
			u8 no_2ndary_sids : 1;
			u8 parse_all_events : 1;
			u8 spc : 1; // Short Packet Capability
			u8 sec : 1; // Stopped EDTLA capability
			u8 cfc : 1; // Contiguous frame ID capability
			u8 maxpsasize : 4; // Maximum Primary Stream Array Size
			u16 extended_caps_offset; // Offszet 32 bites egységekben, nem byte-okban
		};
	};
	u32 dboff; // Offszet 32 bites egységekben, nem byte-okban
	u32 rtsoff; // Na ez már byte offszet xddddddd
	union _attr_packed {
		u32 hccparams2;
		volatile struct _attr_packed {
			u8 u3c : 1;
			u8 cmc : 1; // Configure Endpoint Command Max Exit Latency Too Large Capability
			u8 fsc : 1; // Force save context capability
			u8 ctc : 1; // Compliance Transition Capability
			// TODO
			u32 : 28;
		};
	};
} xhci_caps;

typedef volatile struct _attr_packed xhci_regs {
	union _attr_packed {
		u32 raw;
		volatile struct _attr_packed {
			u32 rs : 1;
			u32 hcreset : 1;
			u32 intr : 1;
			u32 host_err : 1;
			u32 lighthcreset : 1; // HC reset, de a portok érintetlenül maradnak
			u32 css : 1; // Controller Save State
			u32 crs : 1; // Controller Restore State
			u32 ewe : 1; // Enable Wrap Event
			u32 eu3s : 1; // Enable U3 MFINDEX Stop
			u32 : 1;
			u32 cme : 1; // CEM Enable
			u32 tbe : 1; // Extended TBC enable
			u32 tsc_en : 1; // Extended TBC TRB Status Enable
			u32 vtioe : 1; // VTIO Enable
		};
	} usbcmd;
	union _attr_packed {
		u32 raw;
		volatile struct _attr_packed {
			u32 hchalted : 1;
			u32 : 1;
			u32 hse : 1; // Host Sys Err
			u32 eint : 1; // Event Interrupt
			u32 pcd : 1; // Port Change Detect
			u32 sss : 1; // Save State Status
			u32 rss : 1; // Restore State Status
			u32 sre : 1; // Save/Restore Error
			u32 cnr : 1; // Controller Not Ready; amíg nem nulla, nem lehet írni az USBSTS-on kívül sehova se
			u32 hce : 1; // HC Error
			u32 : 22;
		};
	} usbsts;
	u32 pagesize; // Ha bit 'n' 1: 2^(n+12)-ena pagesize; pl. ha ez 0x1, akkor 4k
	u8 r0[8];
	u32 dnctrl; // Device Notification Control
	union _attr_packed {
		u64 crcr; // Command Ring Control
		u64 cmd_ring_ptr; // Alsó biteket (6 db) maszkolni kell
		volatile struct _attr_packed {
			u64 rcs : 1; // Ring Cycle State
			u64 cs : 1; // Command Stop
			u64 ca : 1; // Command Abort
			u64 crr : 1; // Command Ring Running
			u64 : 2;
		};
	};
	u8 r1[16];
	u64 dcbaap; // Device Context Base Address Array Pointer
	volatile struct _attr_packed {
		u32 raw;
		volatile struct _attr_packed {
			u32 max_dev_slots_enabled : 8;
			u32 u3_entry_disable : 1;
			u32 cie : 1;
			u32 : 22;
		};
	} config;
	u8 r2[964];
	volatile struct _attr_packed {
		union _attr_packed {
			u32 portsc;
			volatile struct _attr_packed {

			};
		};
		union _attr_packed {
			u32 portpmsc; // pwr mgmt
			volatile struct _attr_packed {
				
			};
		};
		union _attr_packed {
			u32 portli; // link info
			volatile struct _attr_packed {
				
			};
		};
	} ports[0];
} xhci_regs;

typedef volatile struct _attr_packed xhci_rt_regs {
	u32 mfindex;
	// TODO
} xhci_rt_regs;

typedef struct _attr_packed xhci_trb {
	union _attr_packed {
		struct _attr_packed {
			u64 databuf;
			struct _attr_packed {
				u32 transferlen : 17;
				u32 td_size : 5;
				u32 interrupter_target : 10;
			};
			struct _attr_packed {
				u32 cycle : 1;
				u32 eval_next_trb : 1;
				u32 int_on_short_packet : 1;
				u32 no_snoop : 1;
				u32 chain : 1;
				u32 ioc : 1;
				u32 immediate_data : 1;
				u32 : 2;
				u32 block_event_int : 1;
				u32 type : 6;
				u32 : 16;
			};
		} normal;
		struct _attr_packed {
			u8 bmRequestType;
			u8 bRequest;
			u16 wValue;
			u16 wIndex;
			u16 wLength;
			struct _attr_packed {
				u32 transferlen : 17;
				u32 td_size : 5;
				u32 interrupter_target : 10;
			};
			struct _attr_packed {
				u32 cycle : 1;
				u32 : 4;
				u32 ioc : 1;
				u32 immediate_data : 1;
				u32 : 3;
				u32 type : 6;
				enum {
					NO_DATA_STAGE = (u32)0,
					DATA_OUT = (u32)2,
					DATA_IN = (u32)3,
				} transfer_type : 2;
				u32 : 14;
			};
		} setup;
		struct _attr_packed {
			u64 data;
			struct _attr_packed {
				u32 transferlen : 17;
				u32 td_size : 5;
				u32 interrupter_target : 10;
			};
			struct _attr_packed {
				u32 cycle : 1;
				u32 eval_next_trb : 1;
				u32 int_on_short_packet : 1;
				u32 no_snoop : 1;
				u32 chain : 1;
				u32 ioc : 1;
				u32 immediate_data : 1;
				u32 : 3;
				u32 type : 6;
				u32 direction_in : 1;
				u32 : 15;
			};
		} data;
		struct _attr_packed {
			u64 : 64;
			struct _attr_packed {
				u32 : 22;
				u32 interrupter_target : 10;
			};
			struct _attr_packed {
				u32 cycle : 1;
				u32 eval_next_trb : 1;
				u32 : 2;
				u32 chain : 1;
				u32 ioc : 1;
				u32 : 4;
				u32 type : 6;
				u32 direction_in : 1;
				u32 : 15;
			};
		} status;
	};
} xhci_trb;

typedef struct _attr_packed xhci_dev_context {
	
} xhci_dev_context;

void xhci_init(dev_misc* d);
