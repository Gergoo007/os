#include <usb/hci/xhci.h>
#include <pci/pci.h>
#include <gfx/console.h>
#include <mm/paging.h>

void xhci_init(dev_misc* d) {
	xhci_caps* caps = VIRTUAL((xhci_caps*)
							((u64)pci_bar_addr(0, d->hdr.pci_addr.bus, d->hdr.pci_addr.dev, d->hdr.pci_addr.fun, 0) |
							(u64)(pci_bar_addr(0, d->hdr.pci_addr.bus, d->hdr.pci_addr.dev, d->hdr.pci_addr.fun, 1) << 32)));

	
}
