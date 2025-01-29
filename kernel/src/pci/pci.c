#include <pci/pci.h>
#include <pci/pci_ids.h>
#include <gfx/console.h>
#include <mm/paging.h>
#include <mm/vmm.h>
#include <serial/serial.h>
#include <devmgr/devmgr.h>
#include <util/string.h>
#include <util/mem.h>

mcfg* mc;

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
				bool bus_added = false;
				dev_pcibus* busdev;

				void* bus_address = (void*) (mc->config_spaces[i].base + (bus << 20));
				MAKE_VIRTUAL(bus_address);

				foreach (device, 32) {
					void* dev_addr = (void*) (bus_address + (device << 15));
					pci_hdr* dev = dev_addr;

					if (dev->vendor == 0xffff) continue;

					if (!bus_added) {
						busdev = kmalloc(sizeof(dev_pcibus));
						memset(busdev, 0, sizeof(dev_pcibus));
						busdev->hdr.pci_addr.cfg_space = i;
						busdev->hdr.pci_addr.bus = bus;
						busdev->hdr.type = DEV_PCI_BUS;
						dev_add(computer, busdev);
						bus_added = true;
					}

					foreach (f, 8) {
						pci_hdr* func = (pci_hdr*) (dev_addr + (f << 12));
						if (func->vendor == 0 || func->vendor == 0xffff) continue;

						dev_misc* d = kmalloc(sizeof(dev_misc));
						memset(d, 0, sizeof(dev_misc));
						dev_misc_pci_data* data = kmalloc(sizeof(dev_misc_pci_data));
						for (u32 k = 0; k < 6; k++)
							data->bars[k] = func->type0.bars[k];
						data->hdr_type = func->hdr_type;
						data->vendor = func->vendor;
						data->product = func->product;
						data->prog_if = func->prog_if;
						data->class = func->class;
						data->subclass = func->subclass;
						data->prog_if = func->prog_if;

						d->hdr.type = DEV_PCI_MISC;
						d->hdr.pci_addr.cfg_space = i;
						d->hdr.pci_addr.bus = bus;
						d->hdr.pci_addr.dev = device;
						d->hdr.pci_addr.fun = f;
						d->hdr.handle = (void*)data;
						dev_add(busdev, d);
					}
				}
			}
		}
	} else {
		// Brute force
		for (u32 b = 0; b < 256; b++) {
			bool bus_added = false;
			dev_pcibus* busdev;

			for (u32 d = 0; d < 32; d++) {
				pci_hdr dev;
				dev.dword[0] = pci_io_read_dword(b, d, 0, 0);
				if (dev.vendor == 0xffff) continue;
				dev.dword[1] = pci_io_read_dword(b, d, 0, 4);
				dev.dword[2] = pci_io_read_dword(b, d, 0, 8);

				if (!bus_added) {
					busdev = kmalloc(sizeof(dev_pcibus));
					memset(busdev, 0, sizeof(dev_pcibus));
					busdev->hdr.pci_addr.cfg_space = 0;
					busdev->hdr.pci_addr.bus = b;
					busdev->hdr.type = DEV_PCI_BUS;
					dev_add(computer, busdev);
					bus_added = true;
				}

				for (u32 f = 0; f < 8; f++) {
					pci_hdr fun;
					fun.dword[0] = pci_io_read_dword(b, d, f, 0);
					if (fun.vendor == 0xffff) continue;

					for (u32 i = 1; i < 16; i++) {
						fun.dword[i] = pci_io_read_dword(b, d, f, i*4);
					}

					dev_misc_pci_data* data = kmalloc(sizeof(dev_misc_pci_data));
					for (u32 k = 0; k < 6; k++)
						data->bars[k] = fun.type0.bars[k];
					data->hdr_type = fun.hdr_type;
					data->vendor = fun.vendor;
					data->product = fun.product;
					data->prog_if = fun.prog_if;
					data->class = fun.class;
					data->subclass = fun.subclass;
					data->prog_if = fun.prog_if;

					dev_misc* ddev = kmalloc(sizeof(dev_misc));
					memset(ddev, 0, sizeof(dev_misc));
					ddev->hdr.type = DEV_PCI_MISC;
					ddev->hdr.pci_addr.cfg_space = 0;
					ddev->hdr.pci_addr.bus = b;
					ddev->hdr.pci_addr.dev = d;
					ddev->hdr.pci_addr.fun = f;
					ddev->hdr.handle = data;
					dev_add(busdev, ddev);
				}
			}
		}
	}
}

u64 pci_bar_addr(u16 seg, u8 bus, u8 slot, u8 fun, u8 barnum) {
	u32 bar = pci_readl(seg, bus, slot, fun, 0x10 + barnum * 4);

	u32 lower = bar & ~0b1111;
	if ((bar & 0b0110) == 0) {
		return lower;
	} else if ((bar & 0b0110) == 4) {
		if (barnum == 6)
			return lower;
		else
			return lower | ((pci_readl(seg, bus, slot, fun, 0x10 + (barnum+1) * 4) & ~0b1111ULL) << 32);
	} else {
		warn("PCI bar 0");
		return 0;
	}
}

u8 pci_bar_is_io(u16 seg, u8 bus, u8 slot, u8 fun, u8 barnum) {
	u32 bar = pci_readl(seg, bus, slot, fun, 0x10 + barnum * 4);
	return bar & 1;
}

void pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val) {
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

void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val) {
	pci_writeb(seg, bus, slot, fun, offset, val);
}

void pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val) {
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

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val) {
	pci_writew(seg, bus, slot, fun, offset, val);
}

void pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val) {
	if (mc) {
		volatile u32* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		*addr = val;
	} else {
		pci_io_write_dword(bus, slot, fun, offset, val);
	}
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val) {
	pci_writed(seg, bus, slot, fun, offset, val);
}

uint8_t pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	if (mc) {
		volatile u8* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		return *addr;
	} else {
		return pci_io_read_dword(bus, slot, fun, offset);
	}
}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	return pci_readb(seg, bus, slot, fun, offset);
}

uint16_t pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	if (mc) {
		volatile u16* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		return *addr;
	} else {
		return pci_io_read_dword(bus, slot, fun, offset);
	}
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	return pci_readw(seg, bus, slot, fun, offset);
}

uint32_t pci_readl(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	if (mc) {
		volatile u32* addr = (void*)
			(mc->config_spaces[0].base + (bus << 20) + (slot << 15) + (fun << 12)) + offset;
		MAKE_VIRTUAL(addr);

		return *addr;
	} else {
		return pci_io_read_dword(bus, slot, fun, offset);
	}
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
	return pci_readl(seg, bus, slot, fun, offset);
}
