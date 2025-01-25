#include <usb/hci/uhci.h>
#include <usb/usb.h>
#include <gfx/console.h>
#include <serial/serial.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <arch/arch.h>
#include <util/mem.h>

_attr_pagealigned static usb_request request;

void uhci_start(u16 io) {
	ucmd c = { .ucmd = inw(io + UCMD), };
	c.run = 1;
	outw(c.ucmd, io + UCMD);
}

void uhci_stop(u16 io) {
	ucmd c = { .ucmd = inw(io + UCMD), };
	c.run = 0;
	outw(c.ucmd, io + UCMD);
}

void uhci_reset_port(u16 portio) {
	uport p = { .uport = inw(portio), };
	// Port reset
	p.reset = 1;
	outw(p.uport, portio);
	sleep(50);
	p.reset = 0;
	p.conn_change = 0;
	p.port_en_changed = 0;
	outw(p.uport, portio);

	sleep(10);
}

void uhci_control(u8 pid, u16 portio, uhci* hci, u8 addr, u8 endp, void* outbuf, i32 mps, i8 bytes) {
	uport p = { .uport = inw(portio), };
	u8 data = 0;
	if (!mps) mps = 8;

	memset(hci->tdlist, 0, 0x1000);
	memset(hci->qhlist, 0, 0x1000);

	hci->tdlist[0].buffer = (u32)(u64)paging_lookup((u64)&request);
	hci->tdlist[0].link.link = (u32)((u64)PHYSICAL(&hci->tdlist[1]) | 0b100);
	hci->tdlist[0].ioc = 0; // ha nem jó
	hci->tdlist[0].ls = p.ls;
	hci->tdlist[0].active = 1;
	hci->tdlist[0].addr = addr;
	hci->tdlist[0].endp = endp;
	hci->tdlist[0].data = data;
	hci->tdlist[0].maxlen = 7;
	hci->tdlist[0].pid = UPID_SETUP;

	data = !data;

	u32 i = 1;
	while (bytes > 0) {
		// IN TD-k
		hci->tdlist[i].buffer = (u32)(u64)paging_lookup((u64)outbuf + ((i-1) * 8));
		hci->tdlist[i].link.link = (u32)(u64)PHYSICAL(&hci->tdlist[i+1]) | 0b100;
		hci->tdlist[i].ioc = 0;
		hci->tdlist[i].ls = p.ls;
		hci->tdlist[i].active = 1;
		hci->tdlist[i].addr = addr;
		hci->tdlist[i].endp = endp;
		hci->tdlist[i].data = data;
		// Ez lehet nem is kell? Csak akkor SPD lesz
		if (bytes < mps) {
			hci->tdlist[i].maxlen = bytes - 1;
		} else {
			hci->tdlist[i].maxlen = mps - 1;
		}
		hci->tdlist[i].pid = pid;

		i++;
		data = !data;
		bytes -= mps;
	}

	// STATUS
	hci->tdlist[i].buffer = 0;
	hci->tdlist[i].link.link = 1 | 0b100;
	hci->tdlist[i].ioc = 1;
	hci->tdlist[i].ls = p.ls;
	hci->tdlist[i].active = 1;
	hci->tdlist[i].addr = addr;
	hci->tdlist[i].endp = endp;
	hci->tdlist[i].data = 1;
	hci->tdlist[i].maxlen = 0x7ff;
	hci->tdlist[i].pid = pid == UPID_IN ? UPID_OUT : UPID_IN;

	hci->qhlist[0].head.t = 1;
	hci->qhlist[0].elem.link = (u64)PHYSICAL(hci->tdlist);
	hci->qhlist[0].elem.q = 0;

	hci->frlist[0] = (u64)PHYSICAL(&hci->qhlist[0]) | 0b10;

	uhci_stop(hci->io);
	outw(0, hci->io + UFRNUM);

	uhci_start(hci->io);

	while (~inw(hci->io + USTS) & 1);

	uhci_stop(hci->io);

	outw(0xffff, hci->io + USTS);
}

u8 uhci_init_port(u16 portio, u16 io, dev_misc* pcidev) {
	uport p = { .uport = inw(portio) };

	uhci_reset_port(portio);

	for (u32 i = 0; i < 10; i++) {
		p.uport = inw(portio);

		if (p.conn_change || p.port_en_changed) {
			p.conn_change = 1;
			p.port_en_changed = 1;
		}

		// Van eszköz a porton?
		if (!p.connected) return 0;

		// Enable bit
		if (p.port_enabled) break;

		// Ha még mindig megy a loop
		p.port_enabled = 1;

		outw(p.uport, portio);
	}

	request.raw = USB_GET_DESC_DEVICE;

	if ((u64)PHYSICAL(((uhci*)(pcidev->hdr.handle))->tdlist) & 0xffffffff00000000)
		error("tdlist selejt");

	// Reset recovery
	sleep(10);

	// A protokoll:
	// - Reset
	// - MPS nagyságú GET_DESCRIPTOR DEVICE request
	// - Reset
	// - Set address
	// - GET_DESCRIPTOR DEVICE

	usb_DEVICE dev;
	request.bmRequestType = 0;
	request.bRequest = USB_REQ_SET_ADDRESS;
	request.wIndex = 0;
	request.wValue = 8;
	request.wLength = 0;
	uhci_control(UPID_OUT, portio, pcidev->hdr.handle, 0, 0, &dev, 0, 0);

	request.raw = USB_GET_DESC_DEVICE;
	uhci_control(UPID_IN, portio, pcidev->hdr.handle, 8, 0, &dev, dev.bMaxPacketSize, 18);

	// A stringek bekérése

	// Végül hozzáadom az eszközt a listához
	// dtree_usb_dev udev = {
	// 	.h.num_children = 0,
	// 	.h.parent = pcidev->h.index,
	// 	.vendor = "Hello world",
	// };
	// if (dev.bDeviceClass == 0 && dev.bDeviceSubclass == 0)
	// 	udev.h.type = DEV_USB_HID;
	// else
	// 	udev.h.type = DEV_USB_MISC;
	// dtree_add_usb_dev(&udev);

	return 1;
}

void uhci_init(dev_misc* dev) {
	uhci* handle = kmalloc(sizeof(uhci));
	u16 io = pci_bar_addr(0, dev->hdr.pci_addr.bus, dev->hdr.pci_addr.dev, dev->hdr.pci_addr.fun, 4);

	dev->hdr.handle = handle;
	handle->io = io;
	handle->qhlist = vmm_alloc();
	handle->tdlist = vmm_alloc();

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
	handle->frlist = vmm_alloc();
	for (u32 i = 0; i < 2048; i++) {
		handle->frlist[i] = 1; // Terminate
	}
	outl((u64)PHYSICAL(handle->frlist), io + UFRBASE);

	// SOFMOD
	outb(0x40, io + USOFMOD);

	// Intr
	outw(0xf, io + UINTR);

	if (uhci_init_port(io + UPORTSC1, io, dev)) {
		// printk("%04x %04x\n", ((usb_DEVICE*)out)->idVendor, ((usb_DEVICE*)out)->idProduct);
	}

	if (uhci_init_port(io + UPORTSC2, io, dev)) {
		// printk("%04x %04x\n", ((usb_DEVICE*)out)->idVendor, ((usb_DEVICE*)out)->idProduct);
	}
}
