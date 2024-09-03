#include <util/types.h>
#include <util/attrs.h>
#include <pcie/pcie.h>

enum {
	AHCI_FIS_REG_H2D	= 0x27,
	AHCI_FIS_REG_D2H	= 0x34,
	AHCI_FIS_DMA_ACT	= 0x39,
	AHCI_FIS_DMA_SETUP	= 0x41,
	AHCI_FIS_DATA		= 0x46,
	AHCI_FIS_BIST		= 0x58,
	AHCI_FIS_PIO_SETUP	= 0x5F,
	AHCI_FIS_DEV_BITS	= 0xA1,
};

enum {
	AHCI_SIG_SATA	= 0x00000101,
	AHCI_SIG_SATAPI	= 0xEB140101,
	AHCI_SIG_SEMB	= 0xC33C0101,	// Enclosure management bridge
	AHCI_SIG_PM		= 0x96690101,
};

enum {
	ATA_DEV_BUSY = 0x80,
	ATA_DEV_DRQ = 0x08,
};

// ATA parancsok
enum {
	ATA_CMD_IDENTIFY = 0xec,
	ATA_READ_DMA_EX = 0x25,
};

typedef volatile struct _attr_packed fis_reg_h2d {
	u8 type;

	u8 port_multi : 4;
	u8 : 3;
	u8 cmd_ctrl : 1;

	u8 cmd;
	u8 featurel;

	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 dev;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 featureh;

	u8 countl;
	u8 counth;

	u8 isochronous_complete;
	u8 ctrl;

	u32 : 32;
} fis_reg_h2d;

typedef volatile struct _attr_packed fis_reg_d2h {
	u8 type;

	u8 port_multi : 4;
	u8 : 2;
	u8 intr : 1;
	u8 : 1;

	u8 sts;
	u8 err;

	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 dev;

	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 : 8;

	u8 countl;
	u8 counth;

	u16 : 16;

	u32 : 32;
} fis_reg_d2h;

typedef volatile struct _attr_packed hba_port {
	u32 cmd_listl;
	u32 cmd_listu;

	u32 fisl;
	u32 fisu;

	union _attr_packed {
		u32 value;
		struct _attr_packed {
			u8 d2h : 1;
			u8 pio_setup : 1;
			u8 dma_setup : 1;
			u8 set_dev_bits : 1;
			u8 unknown_fis : 1;
			u8 prdt_finished : 1;
			u8 connection_change : 1;
			u8 mechanical_change : 1;
			u16 : 14;
			u8 phyrdy_change : 1;
			u8 incorrect_pm : 1;
			u8 overflow_sts : 1;
			u8 : 1;
			u8 if_non_fatal : 1;
			u8 if_fatal : 1;
			u8 host_bus_non_fatal : 1;
			u8 host_bus_fatal : 1;
			u8 task_file_error : 1;
			u8 cold_port_detect : 1;
		};
	} int_sts;
	u32 int_enable;

	union _attr_packed {
		u32 value;
		struct _attr_packed {
			u8 start : 1;
			u8 spin_up : 1;
			u8 power_on : 1;
			u8 cmd_list_overr : 1;
			u8 fis_rx_enable : 1;
			u8 : 3;
			u8 current_cmd_slot : 5;
			u8 mpss : 1;
			u8 fis_rx_running : 1;
			u8 cmd_list_running : 1;
			u8 cps : 1;
			u8 pma : 1;
		};
	} cmd;
	u32 : 32;

	u32 task_file_data;
	u32 sig;

	union _attr_packed {
		u32 value;
		struct _attr_packed {
			u8 det : 4;
			u8 spd : 4;
			u8 ipm : 4;
		};
	} ssts;
	u32 sctrl;
	u32 serror;
	u32 sactive;

	u32 cmd_issue;

	u32 snotif;

	u32 fis_based_switch;

	u32 res[15];
} hba_port;

typedef volatile struct _attr_packed hba_mem {
	u32 host_capability;
	u32 global_host_ctrl;
	u32 intr_sts;
	u32 ports_impl;
	u32 ver;
	u32 ccc_ctrl;
	u32 ccc_pts;
	u32 em_loc;
	u32 em_ctrl;
	u32 host_capability2;
	u32 handoff_sts;

	u8 res[0xd4];

	hba_port ports[32];
} hba_mem;

typedef volatile struct _attr_packed hba_cmd_hdr {
	u8 fis_len : 5; // FIS hossz * 32 bit
	u8 satapi : 1;
	u8 write : 1;
	u8 prefetchable : 1;
	u8 reset : 1;
	u8 bist : 1;
	u8 clear_busy_upon_ok : 1;
	u8 : 1;
	u8 pm : 1;

	u16 prdt_num_entries;
	
	volatile u32 prd_bytes_transferred;

	u32 cmd_table_desc_l;
	u32 cmd_table_desc_u;

	u32 _[4];
} hba_cmd_hdr;

typedef volatile struct _attr_packed hba_prdt_entry {
	u32 data_base_addrl;
	u32 data_base_addru;
	u32 : 32;

	u32 num_bytes : 22;
	u32 : 9;
	u32 ioc : 1;
} hba_prdt_entry;

typedef volatile struct _attr_packed hba_cmd_table {
	u8 cmd_fis[64];
	u8 satapi_cmd[16];
	u8 _[48];

	hba_prdt_entry prdt_entry[1]; // Lehet belőle semennyi meg 65535 is
} hba_cmd_table;

typedef struct ahci {
	u32 pi;
} ahci;

typedef struct _attr_packed ata_identity {
	u16 flags;
	u16 _1[9];
	char serial[20];
	u16 _2[3];
	char firmware[8];
	char model[40];
	u16 sectors_per_int;
	u16 : 16;
	union _attr_packed {
		u32 value;
		struct _attr_packed {
			u8 phys_sector_alignment : 2;
			u8 : 6;
			u8 dma_support : 1;
			u8 lba_support : 1;
			u8 io_rdy_disable : 1;
			u8 io_rdy_supported : 1;
			u8 : 1;
			u8 standby_timer_support : 1;
			u8 : 2;
			u16 : 16;
		};
	} caps;
	u32 : 32;
	u16 valid_ext_data;
	u16 _4[5];
	u16 size_of_rw_mult;
	u32 sectors_28;
	u16 _5[38];
	u64 sectors_48;
	u16 _6[152];
} ata_identity;

ata_identity* ahci_identify(hba_port* port);
void ahci_init(pci_hdr* hdr);
void ahci_read(hba_port* port, u64 start, u64 count, void* buf);

