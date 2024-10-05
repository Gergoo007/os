#include <acpi/acpi.h>
#include <acpi/lai/core.h>
#include <acpi/lai/helpers/sci.h>
#include <acpi/lai/helpers/pm.h>

#include <gfx/console.h>
#include <util/string.h>

#include <pci/pci.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/clocks/hpet.h>
#include <arch/x86/clocks/tsc.h>

#include <mm/paging.h>

#include <sysinfo.h>

#include <serial/serial.h>

static void* _list;
static u32 _entries;
static bool _quadptrs;

u8 fadt_extended_addresses = 0;
static fadt* f;

void gas_write(gas* a, u64 val, u32 size) {
	if (a->access_size)
		if (size != (1 << (a->access_size-1)))
			error("gas: size (%d) != access_size (%d)", size, a->access_size);

	if (a->addr_space == GAS_IO) {
		if (size == 8) {
			outl(val, a->address);
			outl(val>>32, a->address+4);
		} else if (size == 4) {
			outl(val, a->address);
		} else if (size == 2) {
			outw(val, a->address);
		} else if (size == 1) {
			outb(val, a->address);
		}
	} else if (a->addr_space == GAS_MMIO) {
		if (size == 8) {
			*(volatile u64*)a->address = val;
		} else if (size == 4) {
			*(volatile u32*)a->address = val;
		} else if (size == 2) {
			*(volatile u16*)a->address = val;
		} else if (size == 1) {
			*(volatile u8*)a->address = val;
		}
	} else {
		error("Ismeretlen GAS címtér: %d", a->addr_space);
	}
}

u64 gas_read(gas* a, u32 size) {
	if (a->access_size)
		if (size != (1 << (a->access_size-1)))
			error("gas: size (%d) != access_size (%d)", size, a->access_size);

	if (a->addr_space == GAS_IO) {
		if (size == 8) {
			u64 val = 0;
			val = inl(a->address);
			val |= (u64)inl(a->address+4) << 32;
			return val;
		} else if (size == 4) {
			return inl(a->address);
		} else if (size == 2) {
			return inw(a->address);
		} else if (size == 1) {
			return inb(a->address);
		}
	} else if (a->addr_space == GAS_MMIO) {
		if (size == 8) {
			return *(volatile u64*)a->address;
		} else if (size == 4) {
			return *(volatile u32*)a->address;
		} else if (size == 2) {
			return *(volatile u16*)a->address;
		} else if (size == 1) {
			return *(volatile u8*)a->address;
		}
	} else {
		error("Ismeretlen GAS címtér: %d", a->addr_space);
	}
	return 0;
}

void* laihost_scan(const char* othersig, size_t index) {
	u32 lentries = _entries;
	u32 sig = *(u32*)othersig;
	if (sig == ACPI_DSDT) {
		if (f->x_dsdt && fadt_extended_addresses) {
			return (void*)VIRTUAL(f->x_dsdt);
		} else {
			return (void*)VIRTUAL((u64)f->dsdt);
		}
	}

	while (lentries--) {
		sdt_hdr* table = _quadptrs ? (sdt_hdr*)*(u64*)(_list+lentries*8) : (sdt_hdr*) (u64)*(u32*)(_list+lentries*4);
		MAKE_VIRTUAL(table);

		if (table->sign == sig) {
			if (index == 0) {
				return table;
			} else {
				index--;
			}
		}
	}

	return 0;
}

u32 acpi_read_timer() {
	if (f->x_pm_timer.address && fadt_extended_addresses)
		return gas_read32(&f->x_pm_timer);
	else if (f->pm_timer)
		return inl(f->pm_timer);
	else
		error("Nincs ACPI timer!");
	return 0;
}

static void process_fadt() {
	if (f->x_pm_timer.address || f->pm_timer) {
		pm_timer_present = 1;
	}
}

void acpi_process_tables(void* list, u32 entries, bool quadptrs) {
	_list = list;
	_entries = entries;

	hpet_table* ht = 0;
	mc = 0;
	madt* ma = 0;
	while (entries--) {
		sdt_hdr* table = quadptrs ? (sdt_hdr*)*(u64*)(list+entries*8) : (sdt_hdr*) (u64)*(u32*)(list+entries*4);
		MAKE_VIRTUAL(table);

		switch (table->sign) {
			case ACPI_MCFG: {
				mc = (mcfg*)table;
				break;
			}

			case ACPI_APIC: {
				ma = (madt*)table;
				break;
			}

			case ACPI_FACP: {
				f = (fadt*)table;
				if (f->h.len > 132)
					fadt_extended_addresses = 1;
				break;
			}

			case ACPI_HPET: {
				ht = (hpet_table*)table;
				break;
			}

			case ACPI_ECDT: {
				report("embedded ctrl..\n");
				break;
			}

			default: {
				break;
			}
		}
	}

	process_fadt();
	apic_process_madt((madt*)ma);

	// hpet_init((hpet_table*)ht);

	pci_init();
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

	if (strcmp((char*)&r->sign, "RSD PTR"))
		error("Érvénytelen RSDP!");

	printk("oem %.6s\n", r->oem);

	if (x)
		acpi_xsdt((xsdt*)r->xsdt);
	else
		acpi_rsdt((rsdt*)(u64)r->rsdt);

	tsc_configure_using_acpi();

	lai_set_acpi_revision(r->rev);
	lai_create_namespace();

	lai_enable_acpi(1);

	// bit 8 & 9
	lai_set_sci_event(0x300);

	ioapic_entry e = { .raw = 0, };
	e.vector = 0x43;
	ioapic_write_entry(f->sci_int, e);
	lai_get_sci_event();
}

u8 acpi_8042_present() {
	return f->pc_feats.ps2ctrl;
}

u8 acpi_vga_present() {
	return !f->pc_feats.no_vga;
}

u8 acpi_isa_lpt_present() {
	return !f->pc_feats.legacy_devs;
}

u8 acpi_rtc_present() {
	return !f->pc_feats.no_rtc;
}

u8 acpi_msi_usable() {
	return !f->pc_feats.no_msi;
}
