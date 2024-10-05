// #include <usb/hci/ehci.h>
// #include <usb/usb.h>
// #include <gfx/console.h>
// #include <arch/x86/clocks/tsc.h>
// #include <mm/vmm.h>
// #include <mm/pmm.h>
// #include <mm/paging.h>
// #include <util/mem.h>
// #include <arch/x86/clocks/pit.h>

// ehci_qh* aqh;
// void* outbuf;

// void ehci_on_device(volatile u32* port);

// void ehci_init(pci_hdr* d) {
// 	u16 cmd = d->cmd;
// 	cmd |= 0b111;
// 	d->cmd = cmd;

// 	if (pci_bar_is_io(d, 0))
// 		error("io ehci\n");

// 	ehci_caps* ecaps = (ehci_caps*)(pci_bar_addr(d, 0) | 0xffff800000000000);
// 	volatile ehci_op_regs* e = (ehci_op_regs*) ((u64)ecaps + ecaps->cap_len);
// 	e->usbcmd |= 0b10;
// 	while (e->usbcmd & 0b10);

// 	u64 a = (u64)pmm_alloc();
// 	if (a >> 32)
// 		error("64 bites cím");
// 	e->frbase = (u32)a;

// 	// printk("Companion vezérlők: %d\n", ecaps->structparams.num_comp);
// 	// printk("Comp. vezérlő portok: %d\n", ecaps->structparams.num_comp_ports);
// 	// printk("Áram: %d\n", ecaps->structparams.port_pwr_ctl);

// 	// Aszinkronizált queue
// 	// TODO: 4 KiB overkill, kmalloc nem jó alignment miatt
// 	aqh = vmm_alloc();
// 	memset(aqh, 0, sizeof(ehci_qh));
// 	aqh->horizontal = (u32)(u64)aqh | PTR_QH;
// 	aqh->endpch = QH_CH_H;
// 	aqh->next_qtd = PTR_TERM;
// 	aqh->alt_next_qtd = PTR_TERM;

// 	// Ismétlődő queue (periodic)
// 	// TODO: 4 KiB overkill, kmalloc nem jó alignment miatt
// 	ehci_qh* pqh = vmm_alloc();
// 	memset(pqh, 0, sizeof(ehci_qh));
// 	pqh->horizontal = PTR_TERM;
// 	pqh->next_qtd = PTR_TERM;

// 	// Frame list kitöltés
// 	u32* frlist = (u32*)((u64)e->frbase | 0xffff800000000000);
// 	for (u32 i = 0; i < 1024; i++) {
// 		frlist[i] = (u64)PHYSICAL(pqh) | PTR_QH;
// 	}

// 	// TODO: BIOS legacy kikapcs.

// 	// Interruptok nem kellenek egyelőre
// 	e->usbint = 0b11;

// 	// Frame list
// 	e->frindex = 0;
// 	e->asynclistbase = (u32)(u64)PHYSICAL(aqh);

// 	e->segment = 0;

// 	// Állapot tisztítása
// 	e->usbsts = 0x3f;

// 	// Vezérlő indítása
// 	cmd = e->usbcmd;
// 	cmd |= 1;
// 	cmd |= (1 << 4);
// 	cmd |= (1 << 5);
// 	e->usbcmd = cmd;

// 	e->configflag = 1;
// 	sleep(5);

// 	// Aszinkronizált sor: control és bulk komm.
// 	// Ismétlődő sor: interrupt és isochronous komm.

// 	// Portok vizsgálata
// 	volatile u32* ports = (u32*)e->ports;
// 	for (u32 i = 0; i < ecaps->structparams.num_ports; i++) {
// 		// Reset
// 		ports[i] |= (1 << 8);
// 		sleep(50);
// 		ports[i] &= (1 << 8);
// 		sleep(50);

// 		for (u32 tries = 0; tries < 3; tries++) {
// 			if (ports[i] & 1) {
// 				if (ports[i] & PORT_ENABLE)
// 					printk("high speed\n");
// 				else
// 					printk("low speed\n");
// 				ehci_on_device(ports + i);
// 				break;
// 			}
// 			// sleep(10);
// 		}
// 	}

// 	printk("usbsts: %02x\n", e->usbsts);
// }

// void ehci_on_device(volatile u32* port) {
// 	printk("DEVICE FOUND\n");

// 	usb_request req = {
// 		.bmRequestType = 0b10000000,
// 		.bRequest = 6,
// 		.wValue = 1 << 8,
// 		.wIndex = 0,
// 		.wLength = 8,
// 	};
// 	void* setup_data = vmm_alloc();
// 	outbuf = vmm_alloc();
// 	memcpy(setup_data, &req, sizeof(usb_request));

// 	ehci_td* td0 = vmm_alloc(),* td1 = vmm_alloc(),* td2 = vmm_alloc();
// 	memset(td0, 0, 0x1000);
// 	memset(td1, 0, 0x1000);
// 	memset(td2, 0, 0x1000);

// 	u8 data = 0;

// 	td0->alt_next_qtd = PTR_TERM;
// 	td0->next_qtd = PTR_TERM;
// 	td0->token =
// 		(data << TD_TOK_D_SHIFT) |
// 		(8 << TD_TOK_LEN_SHIFT) |
// 		(3 << TD_TOK_CERR_SHIFT) |
// 		(USB_PACKET_SETUP << TD_TOK_PID_SHIFT) |
// 		TD_TOK_ACTIVE |
// 		TD_TOK_IOC;

// 	ehci_qh* qh = vmm_alloc();
// 	memset(qh, 0, sizeof(ehci_qh));
// 	int ch =
// 		(8 << QH_CH_MPL_SHIFT) |
// 		QH_CH_DTC |
// 		(0b10 << QH_CH_EPS_SHIFT) |
// 		(0 << QH_CH_ENDP_SHIFT) |
// 		0;
// 	u32 caps =
// 		(1 << QH_CAP_MULT_SHIFT);

// 	ch |= 5 << QH_CH_NAK_RL_SHIFT;

// 	qh->endpch = ch;
// 	qh->endpcaps = caps;

// 	qh->next_qtd = (u64)td0;
// 	qh->alt_next_qtd = PTR_TERM;
// 	qh->token = 0;

// 	qh->horizontal = (u64)PHYSICAL(aqh) | PTR_QH;
// 	aqh->horizontal = (u64)PHYSICAL(qh) | PTR_QH;

// 	sleep(100);

// 	printk("Active: %d\n", td0->token & TD_TOK_ACTIVE);
// }
