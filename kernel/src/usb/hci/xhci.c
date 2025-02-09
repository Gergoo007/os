#include <usb/hci/xhci.h>
#include <pci/pci.h>
#include <gfx/console.h>
#include <mm/paging.h>
#include <arch/arch.h>

void xhci_init(dev_misc* d) {
	volatile xhci_caps* caps = VIRTUAL((volatile xhci_caps*)
							((u64)pci_bar_addr(0, d->hdr.pci_addr.bus, d->hdr.pci_addr.dev, d->hdr.pci_addr.fun, 0) |
							(u64)(pci_bar_addr(0, d->hdr.pci_addr.bus, d->hdr.pci_addr.dev, d->hdr.pci_addr.fun, 1) << 32)));
	void* extcaps = (void*)caps + caps->extended_caps_offset;
	
	volatile xhci_regs* regs = (void*)caps + caps->caplength;
	regs->usbcmd.hcreset = 1;
	while (regs->usbsts.cnr);

	// regs->config.max_dev_slots_enabled = caps->maxslots;
	regs->config.max_dev_slots_enabled = 4;
	while (regs->usbsts.cnr);

	
}
