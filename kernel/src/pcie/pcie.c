#include <pcie/pcie.h>
#include <pcie/pcie_ids.h>
#include <gfx/console.h>
#include <storage/ahci/ahci.h>
#include <mm/paging.h>
#include <usb/hci/uhci.h>
#include <serial/serial.h>

mcfg* m;

void pcie_add_device(pci_hdr* d) {
	report("Device found (%s), %s:%04x", pci_class(d->class), pci_vendor(d->vendor), d->product);

	switch (d->combclass) {
		case PCIE_SATA:
			ahci_init(d);
			break;

		case PCIE_USB:
			if (d->prog_if == 0x00) {
				uhci_init(d);
			}
			break;
	}
}

void pcie_init(mcfg* _m) {
	m = _m;
	u32 num_config_spaces = (m->h.len - (sizeof(mcfg))) / 16;

	foreach (i, num_config_spaces) {
		for (u32 bus = m->config_spaces[i].bus_start; bus < m->config_spaces[i].bus_end; bus++) {
			void* bus_address = (void*) (m->config_spaces[i].base + (bus << 20));
			MAKE_VIRTUAL(bus_address);

			foreach (device, 32) {
				void* dev_addr = (void*) (bus_address + (device << 15));
				pci_hdr* dev = dev_addr;

				if (dev->vendor == 0xffff) continue;

				foreach (f, 8) {
					pci_hdr* func = (pci_hdr*) (dev_addr + (f << 12));
					
					if (func->vendor == 0 || func->vendor == 0xffff) continue;

					pcie_add_device(func);
				}
			}
		}
	}
}

void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val) {
	volatile u8* addr = (void*)
		(m->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
	MAKE_VIRTUAL(addr);

	*addr = val;
}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val) {
	volatile u16* addr = (void*)
		(m->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
	MAKE_VIRTUAL(addr);

	*addr = val;
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val) {
	volatile u32* addr = (void*)
		(m->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
	MAKE_VIRTUAL(addr);

	*addr = val;
}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	volatile u8* addr = (void*)
		(m->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
	MAKE_VIRTUAL(addr);

	return *addr;
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	volatile u16* addr = (void*)
		(m->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
	MAKE_VIRTUAL(addr);

	return *addr;
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	volatile u32* addr = (void*)
		(m->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
	MAKE_VIRTUAL(addr);

	return *addr;
}
