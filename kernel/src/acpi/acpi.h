#pragma once

#include <util/types.h>
#include <util/attrs.h>

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	enum {
		ACPI_WAET = 'TEAW',
		ACPI_HPET = 'TEPH',
		ACPI_MADT = 'TDAM',
		ACPI_MCFG = 'GFCM',
		ACPI_SSDT = 'TDSS',
		ACPI_FACP = 'PCAF',
		ACPI_APIC = 'CIPA',
		ACPI_DSDT = 'TDSD',
		ACPI_XSDT = 'TDSX',
		ACPI_RSDT = 'TDSR',
	};
#else
	enum {
		ACPI_WAET = 'WAET',
		ACPI_HPET = 'HPET',
		ACPI_MADT = 'MADT',
		ACPI_MCFG = 'MCFG',
		ACPI_SSDT = 'SSDT',
		ACPI_FACP = 'FACP',
		ACPI_APIC = 'APIC',
		ACPI_DSDT = 'DSDT',
		ACPI_XSDT = 'XSDT',
		ACPI_RSDT = 'RSDT',
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
	MADT_LX2APIC = 9,
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
			struct {
				u8 acpi_cpu_id;
				u8 apic_id;
				u8 cpu_enabled : 1;
				u8 cpu_online_capable : 1;
				u32 : 30;
			} e0;

			struct {
				u8 ioapic_id;
				u8 : 8;
				u32 ioapic_addr;
				u32 gsi_base;
			} e1;

			struct {
				u8 bus_src;
				u8 irq_src;
				u32 gsi;
				u16 flags;
			} e2;

			struct {
				u8 nmi_src;
				u8 : 8;
				u16 flags;
				u32 gsi;
			} e3;

			struct {
				u8 acpi_cpu_id;
				u16 flags;
				u8 lint;
			} e4;

			struct {
				u16 : 16;
				u64 lapic;
			} e5;

			struct {
				u16 : 16;
				u32 lx2apic_id;
				u32 flags;
				u32 acpi_id;
			} e9;
		};
	} _attr_packed entries[0];
} madt;

void acpi_init(void* boot_info);
void acpi_rsdt(rsdt* r);
void acpi_xsdt(xsdt* x);
