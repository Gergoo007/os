#pragma once

#include <util/types.h>
#include <util/attrs.h>

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	enum {
		ACPI_WAET = 'TEAW',
		ACPI_HPET = 'TEPH',
		ACPI_MCFG = 'GFCM',
		ACPI_SSDT = 'TDSS',
		ACPI_FACP = 'PCAF',
		ACPI_APIC = 'CIPA',
		ACPI_DSDT = 'TDSD',
		ACPI_XSDT = 'TDSX',
		ACPI_RSDT = 'TDSR',
		ACPI_ECDT = 'TDCE',
	};
#else
	enum {
		ACPI_WAET = 'WAET',
		ACPI_HPET = 'HPET',
		ACPI_MCFG = 'MCFG',
		ACPI_SSDT = 'SSDT',
		ACPI_FACP = 'FACP',
		ACPI_APIC = 'APIC',
		ACPI_DSDT = 'DSDT',
		ACPI_XSDT = 'XSDT',
		ACPI_RSDT = 'RSDT',
		ACPI_ECDT = 'ECDT',
	};
#endif

typedef struct _attr_packed sdt_hdr {
	u32 sign;
	u32 len;
	u8 rev;
	u8 checksum;
	char oemid[6];
	char oemtableid[8];
	u32 oemrev;
	u32 creator_id;
	u32 creator_rev;
} sdt_hdr;

enum {
	GAS_MMIO = 0,
	GAS_IO = 1,
};

typedef struct _attr_packed gas {
	u8 addr_space; // 0 - sys mem, 1 - io mem
	u8 reg_bit_width;
	u8 reg_bit_offset;
	u8 access_size;
	u64 address;
} gas;

typedef struct _attr_packed rsdp {
	u64 sign;
	u8 checksum;
	char oem[6];
	u8 rev;
	u32 rsdt;

	u32 len;
	u64 xsdt;
	u8 ext_checksum;
	u32 : 24;
} rsdp;

typedef struct _attr_packed rsdt {
	sdt_hdr h;
	u32 tables[0];
} rsdt;

typedef struct _attr_packed xsdt {
	sdt_hdr h;
	sdt_hdr* tables[0];
} xsdt;

typedef struct _attr_packed mcfg {
	sdt_hdr h;
	u64 : 64;

	struct {
		u64 base;
		u16 seg_group_num;
		u8 bus_start;
		u8 bus_end;
		u32 : 32;
	} _attr_packed config_spaces[0];
} mcfg;

enum {
	MADT_LAPIC = 0,
	MADT_IOAPIC = 1,
	MADT_IOAPIC_OVERRIDE = 2,
	MADT_IOAPIC_NMI = 3,
	MADT_LAPIC_NMI = 4,
	MADT_LAPIC_ADDR = 5,
	MADT_X2APIC = 9,
};

typedef struct _attr_packed madt {
	sdt_hdr h;
	u32 lapic;
	u8 pic_present : 1;
	u32 : 31;

	struct {
		u8 type;
		u8 len;

		union {
			struct _attr_packed {
				u8 acpi_cpu_id;
				u8 apic_id;
				u8 cpu_enabled : 1;
				u8 cpu_online_capable : 1;
				u32 : 30;
			} e_lapic;

			struct _attr_packed {
				u8 ioapic_id;
				u8 : 8;
				u32 ioapic_addr;
				u32 gsi_base;
			} e_ioapic;

			struct _attr_packed {
				u8 bus_src;
				u8 irq_src;
				u32 gsi;
				u16 flags;
			} e_ioapic_overr;

			struct _attr_packed {
				u8 nmi_src;
				u8 : 8;
				u16 flags;
				u32 gsi;
			} e_nmi_ioapic;

			struct _attr_packed {
				u8 acpi_cpu_id;
				u16 flags;
				u8 lint;
			} e_nmi_lapic;

			struct _attr_packed {
				u16 : 16;
				u64 lapic;
			} e_lapic_overr;

			struct _attr_packed {
				u16 : 16;
				u32 x2apic_id;
				u32 flags;
				u32 acpi_id;
			} e_x2apic;
		};
	} _attr_packed firstent;
} madt;

typedef struct _attr_packed fadt {
	sdt_hdr h;

	u32 fw_ctl;
	u32 dsdt;
	u8 : 8;
	u8 preferred_pm_prof;
	u16 sci_int;
	u32 smi_cmd_port;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 s4bios_req;
	u8 pstate_cnt;
	u32 pm1a_evt_blk;
	u32 pm1b_evt_blk;
	u32 pm1a_ctl_blk;
	u32 pm1b_ctl_blk;
	u32 pm2_ctl_blk;
	u32 pm_timer;
	u32 gpe0;
	u32 gpe1;
	u32 : 24;
	u8 pm_timer_bytes;
	u8 r0[17];
	struct _attr_packed {
		u8 legacy_devs : 1;
		u8 ps2ctrl : 1;
		u8 no_vga : 1;
		u8 no_msi : 1;
		u8 no_aspm : 1;
		u8 no_rtc : 1;
		u16 : 10;
	} pc_feats;
	u8 r2;
	union _attr_packed {
		struct _attr_packed {
			u8 wbinvd : 1;
			u8 wbinvd_flush : 1;
			u8 proc_c1 : 1;
			u8 p_lvl2_up : 1;
			u8 power_btn : 1;
			u8 sleep_btn : 1;
			u8 fix_rtc : 1;
			u8 rtc_s4 : 1;
			u8 tmr_val_ext : 1;
			u8 dck_cap : 1;
			u8 reset_reg_sup : 1;
			u8 sealed_case : 1;
			u8 headless : 1;
			u8 cpu_sw_slp : 1;
			u8 pci_exp_wak : 1;
		};
		u32 raw;
	} flags;
	u8 r1[15];
	u8 fadt_minor;
	u64 x_fw_ctl;
	u64 x_dsdt;
	gas x_pm1a_evt_blk;
	gas x_pm1b_evt_blk;
	gas x_pm1a_ctl_blk;
	gas x_pm1b_ctl_blk;
	gas x_pm2_ctl_blk;
	gas x_pm_timer;
	gas x_gpe0;
	gas x_gpe1;
	gas sleep_ctl;
	gas sleep_sts;
	char hypervisor_id[8];
} fadt;

typedef struct _attr_packed hpet_table {
	sdt_hdr h;

	u8 hw_rev;
	u8 comparators : 5;
	u8 counter_64 : 1;
	u8 : 1;
	u8 legacy : 1;
	u16 vendor;
	gas addr;
	u8 hpet_num;
	u16 min_tick;
	u8 page_prot;
} hpet_table;

#define gas_write8(a, v) gas_write(a, v, 1)
#define gas_write16(a, v) gas_write(a, v, 2)
#define gas_write32(a, v) gas_write(a, v, 4)
#define gas_write64(a, v) gas_write(a, v, 8)

#define gas_read8(a) gas_read(a, 1)
#define gas_read16(a) gas_read(a, 2)
#define gas_read32(a) gas_read(a, 4)
#define gas_read64(a) gas_read(a, 8)

void acpi_init(void* boot_info);
void acpi_rsdt(rsdt* r);
void acpi_xsdt(xsdt* x);
u32 acpi_read_timer();
void gas_write(gas* a, u64 val, u32 size);
u64 gas_read(gas* a, u32 size);

u8 acpi_8042_present();
u8 acpi_vga_present();
u8 acpi_isa_lpt_present();
u8 acpi_rtc_present();
u8 acpi_msi_usable();
