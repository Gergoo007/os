#include <pci/pci.h>
#include <pci/pci_ids.h>
#include <gfx/console.h>
#include <storage/ahci/ahci.h>
#include <mm/paging.h>
#include <usb/hci/uhci.h>
#include <usb/hci/ehci.h>
#include <serial/serial.h>

mcfg* mc;

void pci_add_device(pci_hdr* d) {
	report("Device found (%s), %s:%04x", pci_class(d->class), pci_vendor(d->vendor), d->product);

	switch (d->combclass) {
		case PCI_SATA:
			ahci_init(d);
			break;

		case PCI_USB:
			if (d->prog_if == 0x00) {
				uhci_init(d);
			} else if (d->prog_if == 0x20) {
				ehci_init(d);
			}
			break;
	}
}

u32 pci_io_read_dword(u32 bus, u32 dev, u32 func, u8 offset) {
	u32 addr = (u32)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC) | (1 << 31));
	outl(addr, PCI_CFG_ADDR);
	return inl(PCI_CFG_DATA);
}

void pci_io_write_dword(u32 bus, u32 dev, u32 func, u8 offset, u32 value) {
	u32 addr = (u32)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC) | (1 << 31));
	outl(addr, PCI_CFG_ADDR);
	outl(value, PCI_CFG_DATA);
}

void pci_init() {
	if (mc) {
		u32 num_config_spaces = (mc->h.len - (sizeof(mcfg))) / 16;

		foreach (i, num_config_spaces) {
			for (u32 bus = mc->config_spaces[i].bus_start; bus < mc->config_spaces[i].bus_end; bus++) {
				void* bus_address = (void*) (mc->config_spaces[i].base + (bus << 20));
				MAKE_VIRTUAL(bus_address);

				foreach (device, 32) {
					void* dev_addr = (void*) (bus_address + (device << 15));
					pci_hdr* dev = dev_addr;

					if (dev->vendor == 0xffff) continue;

					foreach (f, 8) {
						pci_hdr* func = (pci_hdr*) (dev_addr + (f << 12));
						if (func->vendor == 0 || func->vendor == 0xffff) continue;

						pci_add_device(func);
					}
				}
			}
		}
	} else {
		// Brute force
		for (u32 b = 0; b < 256; b++) {
			for (u32 d = 0; d < 32; d++) {
				pci_hdr dev;
				dev.dword[0] = pci_io_read_dword(b, d, 0, 0);
				if (dev.vendor == 0xffff) continue;
				dev.dword[1] = pci_io_read_dword(b, d, 0, 4);
				dev.dword[2] = pci_io_read_dword(b, d, 0, 8);

				for (u32 f = 0; f < 8; f++) {
					pci_hdr fun;
					fun.dword[0] = pci_io_read_dword(b, d, f, 0);
					if (fun.vendor == 0xffff) continue;
					
					for (u32 i = 1; i < 16; i++) {
						fun.dword[i] = pci_io_read_dword(b, d, f, i*4);
					}

					pci_add_device(&fun);
				}
			}
		}
	}
}

u64 pci_bar_addr(pci_hdr* d, u8 bar) {
	u32 lower = d->type0.bars[bar] & ~0b1111;
	if ((d->type0.bars[bar] & 0b0110) == 0)
		return lower;
	else if ((d->type0.bars[bar] & 0b0110) == 2)
		return lower | (((u64)d->type0.bars[bar+1] & ~0b1111ULL) << 32);
	else
		return 0;
}

u8 pci_bar_is_io(pci_hdr* d, u8 bar) {
	return d->type0.bars[bar] & 1;
}

void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val) {
	if (mc) {
		volatile u8* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		*addr = val;
	} else {
		u32 v = pci_io_read_dword(bus, slot, fun, offset);
		v &= 0xff;
		v |= val;
		pci_io_write_dword(bus, slot, fun, offset, v);
	}
}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val) {
	if (mc) {
		volatile u16* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		*addr = val;
	} else {
		u32 v = pci_io_read_dword(bus, slot, fun, offset);
		v &= 0xffff;
		v |= val;
		pci_io_write_dword(bus, slot, fun, offset, v);
	}
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val) {
	if (mc) {
		volatile u32* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		*addr = val;
	} else {
		pci_io_write_dword(bus, slot, fun, offset, val);
	}
}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	if (mc) {
		volatile u8* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		return *addr;
	} else {
		return pci_io_read_dword(bus, slot, fun, offset);
	}
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	if (mc) {
		volatile u16* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		return *addr;
	} else {
		return pci_io_read_dword(bus, slot, fun, offset);
	}
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	if (mc) {
		volatile u32* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		return *addr;
	} else {
		return pci_io_read_dword(bus, slot, fun, offset);
	}
}
