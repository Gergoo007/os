#include <pcie/pcie.h>
#include <pcie/pcie_ids.h>

#include <gfx/console.h>

#include <storage/ahci/ahci.h>

#include <mm/paging.h>

void pcie_add_device(pci_hdr* d) {
	report("Device found (%s), %s:%04x", pci_class(d->class), pci_vendor(d->vendor), d->product);
	if (d->class == 0x01 && d->subclass == 0x06) {
		ahci_init(d);
	}
}

void pcie_init(mcfg* m) {
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
