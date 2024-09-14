#include <acpi/acpi.h>
#include <acpi/lai/core.h>

#include <gfx/console.h>
#include <util/string.h>

#include <pcie/pcie.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/clocks/hpet.h>

#include <mm/paging.h>

#include <sysinfo.h>

#include <serial/serial.h>

static void* _list;
static u32 _entries;
static bool _quadptrs;

void* laihost_scan(const char* othersig, size_t index) {
	u32 lentries = _entries;
	u32 sig = *(u32*)othersig;

	while (lentries--) {
		sdt_hdr* table = _quadptrs ? (sdt_hdr*)*(u64*)(_list+lentries*8) : (sdt_hdr*) (u64)*(u32*)(_list+lentries*4);
		MAKE_VIRTUAL(table);

		if (table->sign == sig) {
			if (index == 0) {
				return table;
			} else {
				index--;
			}
		} else if (table->sign == ACPI_FACP && sig == ACPI_DSDT) {
			void* dsdt = (void*) (*(u64*)((u64)table+140) | 0xffff800000000000);
			return dsdt;
		}
	}

	return 0;
}

void acpi_process_tables(void* list, u32 entries, bool quadptrs) {
	_list = list;
	_entries = entries;

	hpet_table* ht;
	while (entries--) {
		sdt_hdr* table = quadptrs ? (sdt_hdr*)*(u64*)(list+entries*8) : (sdt_hdr*) (u64)*(u32*)(list+entries*4);
		MAKE_VIRTUAL(table);

		switch (table->sign) {
			case ACPI_MCFG: {
				pcie_init((mcfg*)table);
				break;
			}

			case ACPI_APIC: {
				apic_process_madt((madt*)table);
				break;
			}

			case ACPI_FACP: {
				// fadt* f = (fadt*)table;
				
				break;
			}

			case ACPI_HPET: {
				ht = (hpet_table*)table;
				break;
			}

			default: {
				break;
			}
		}
	}

	// hpet_init((hpet_table*)ht);
}

void acpi_xsdt(xsdt* x) {
	MAKE_VIRTUAL(x);
	if (x->h.sign != ACPI_XSDT) printk("Korrupt XSDT!\n");

	u32 entries = (x->h.len - sizeof(sdt_hdr)) / 8;
	acpi_process_tables((u64*)x->tables, entries, 1);
}

void acpi_rsdt(rsdt* r) {
	MAKE_VIRTUAL(r);
	if (r->h.sign != ACPI_RSDT) printk("Korrupt RSDT!\n");

	u32 entries = (r->h.len - sizeof(sdt_hdr)) / 4;
	acpi_process_tables((u64*)r->tables, entries, 0);
}

void acpi_init(void* boot_info) {
	boot_info += 8;
	u8 x = 0;
	rsdp* r;
	while (*(u32*)boot_info != 0 && *(u32*)(boot_info+4) != 8) {
		if (*(u32*)boot_info == 14)
			r = boot_info+8;

		if (*(u32*)boot_info == 15) {
			r = boot_info+8;
			x = 1;
		}

		boot_info += *(u32*)(boot_info+4);
		if ((u64)boot_info & 7)
			boot_info = (void*)(((u64)boot_info | 0b111) + 1);
	}

	printk("oem %.6s\n", r->oem);

	if (x)
		acpi_xsdt((xsdt*)r->xsdt);
	else
		acpi_rsdt((rsdt*)(u64)r->rsdt);

	lai_set_acpi_revision(r->rev);
	lai_create_namespace();
	
	
}
