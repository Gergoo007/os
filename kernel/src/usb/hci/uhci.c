#include <usb/hci/uhci.h>
#include <usb/usb.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <arch/x86/clocks/pit.h>
#include <util/mem.h>
#include <sysinfo.h>

u32* frlist;
void* out;

uhci_td* tdlist;
uhci_qh* qhlist;

/* _attr_pagealigned */ static usb_request request;

void uhci_send_in(u32 io, u8 addr, u8 endp, void* outbuf, i8 bytes) {
	uport p = { .uport = inw(io), };
	u8 data = 0;

	tdlist[0].buffer = (u32)(u64)paging_lookup((u64)&request);
	tdlist[0].link.link = (u32)((u64)PHYSICAL(tdlist+1) | 0b100);
	tdlist[0].ioc = 0; // ha nem jó
	tdlist[0].ls = p.ls;
	tdlist[0].active = 1;
	tdlist[0].addr = data;
	tdlist[0].endp = endp;
	tdlist[0].data = addr;
	tdlist[0].maxlen = 7;
	tdlist[0].pid = UPID_SETUP;

	data ^= 1;

	u32 i = 1;
	while (bytes > 0) {
		// IN TD-k
		tdlist[i].buffer = (u32)(u64)PHYSICAL(out + ((i-1) * 8));
		tdlist[i].link.link = (u32)(u64)PHYSICAL(&tdlist[i+1]) | 0b100;
		tdlist[i].ioc = 0;
		tdlist[i].ls = p.ls;
		tdlist[i].active = 1;
		tdlist[i].addr = addr;
		tdlist[i].endp = endp;
		tdlist[i].data = data;
		tdlist[i].maxlen = 7;
		tdlist[i].pid = UPID_IN;

		i++;
		data ^= 1;
		bytes -= 8;
	}

	// STATUS
	tdlist[i].buffer = 0;
	tdlist[i].link.link = 1 | 0b100;
	tdlist[i].ioc = 1;
	tdlist[i].ls = p.ls;
	tdlist[i].active = 1;
	tdlist[i].addr = addr;
	tdlist[i].endp = endp;
	tdlist[i].data = !data;
	tdlist[i].maxlen = 0x7ff;
	tdlist[i].pid = UPID_OUT;
}

void uhci_init_port(u32 io) {
	uport p = { .uport = inw(io) };
	p.port_enabled = 1;
	outw(p.uport, io);
	sleep(50);
	if (!p.connected) return;

	p.uport = inw(io);
	p.reset = 1;
	outw(p.uport, io);
	sleep(10);
	p.reset = 0;
	outw(p.uport, io);

	uport t = { .uport = inw(io + UPORTSC1), };
	printk("port %04x\n", t.port_enabled);

	// test
	// usb_request* req = vmm_alloc();
	// req->bmRequestType = 0x80;
	// req->bRequest = 6;
	// req->wValue = (1 << 8);
	// req->wIndex = 0;
	// req->wLength = 18;
	request.raw = USB_GET_DESC_DEVICE;

	if ((u64)PHYSICAL(tdlist) & 0xffffffff00000000)
		error("tdlist selejt");

	uhci_send_in(io, 0, 0, out, 18);

	qhlist[0].head.t = 1;
	qhlist[0].elem.link = (u64)PHYSICAL(tdlist);
	qhlist[0].elem.q = 0;

	frlist[0] = (u64)PHYSICAL(&qhlist[0]) | 0b10;
}

void uhci_init(pci_hdr* d) {
	u16 io = pci_bar_addr(d, 4);

	out = vmm_alloc();
	memset(out, 0, 0x1000);

	tdlist = vmm_alloc();
	memset(tdlist, 0, 0x1000);

	qhlist = vmm_alloc();
	memset(qhlist, 0, 0x1000);

	uhcis[num_uhcis].io = io;
	uhcis[num_uhcis].qhlist = qhlist;
	uhcis[num_uhcis].tdlist = tdlist;
	num_uhcis++;

	// Reset
	ucmd cmd = { .ucmd = inw(io + UCMD) };
	cmd.hcreset = 1;
	outw(cmd.ucmd, io + UCMD);
	while (inw(io + UCMD) & 0b10);

	// Megállítás
	cmd.ucmd = inw(io + UCMD);
	cmd.run = 0;
	outw(cmd.ucmd, io + UCMD);

	// Hibakódok törlése
	outw(0xffff, io + USTS);

	// Int on complete
	outw(1 << 2, io + UINTR);

	// Frame lista
	outw(0x0000, io + UFRNUM);
	frlist = vmm_alloc();
	for (u32 i = 0; i < 2048; i++) {
		frlist[i] = 1; // Terminate
	}
	outl((u64)PHYSICAL(frlist), io + UFRBASE);

	// SOFMOD
	outb(0x40, io + USOFMOD);

	// Intr
	outw(0xf, io + UINTR);

	uport p1 = { .uport = inw(io + UPORTSC1) };
	uport p2 = { .uport = inw(io + UPORTSC2) };

	if (p1.connected)
		uhci_init_port(io + UPORTSC1);

	if (p2.connected)
		uhci_init_port(io + UPORTSC2);

	outw(0xffff, io + USTS);

	cmd.ucmd = inw(io + UCMD);
	cmd.run = 1;
	outw(cmd.ucmd, io + UCMD);

	while (!(inw(io + USTS) & 1));
	sleep(50);

	usb_DEVICE* r = out;
	printk("vendprod %04x %04x, usb %02x\n", r->idVendor, r->idProduct, r->bsdUSB);
}
