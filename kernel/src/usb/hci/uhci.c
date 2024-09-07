#include <usb/hci/uhci.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

void uhci_init_port(u32 port) {
	uport p = { .uport = inw(port) };
	p.port_enabled = 1;
}

void uhci_init(pci_hdr* d) {
	u16 io = d->type0.bar4;
	
	// Reset + megállítás
	ucmd cmd = { .ucmd = inw(io + UCMD) };
	cmd.greset = 1;
	cmd.hcreset = 1;
	cmd.run = 0;
	outw(cmd.ucmd, io + UCMD);

	// Hibakódok törlése
	outw(0xffff, io + USTS);

	// Int on complete
	outw(1 << 2, io + USTS);

	// Frame lista
	outw(0x0000, io + UFRNUM);
	outl((u32)(u64)pmm_alloc(), io + UFRBASE);

	// SOFMOD
	outb(0x40, io + USOFMOD);

	for (u16 i = 1; i; i++);

	uport p1 = { .uport = inw(io + UPORTSC1) };
	uport p2 = { .uport = inw(io + UPORTSC2) };

	if (p1.connected)
		uhci_init_port(io + UPORTSC1);

	if (p2.connected)
		uhci_init_port(io + UPORTSC2);
}
