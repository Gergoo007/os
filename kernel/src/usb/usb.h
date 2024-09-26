#pragma once

#include <util/types.h>
#include <util/attrs.h>

enum {
	USB_H2D = 	0b00000000,
	USB_D2H = 	0b10000000,
	USB_STD = 	0b00000000,
	USB_CLASS = 0b00100000,
	USB_VENDR = 0b01000000,
	USB_DEV =	0b00000000,
	USB_INTF = 	0b00000001,
	USB_ENDP = 	0b00000010,
	USB_OTHR =	0b00000011,
};

enum {
	USB_REQ_GET_STATUS			= 0,
	USB_REQ_CLEAR_FEATURE		= 1,
	USB_REQ_SET_FEATURE	 		= 3,
	USB_REQ_SET_ADDRESS	 		= 5,
	USB_REQ_GET_DESCRIPTOR	 	= 6,
	USB_REQ_SET_DESCRIPTOR	 	= 7,
	USB_REQ_GET_CONFIGURATION	= 8,
	USB_REQ_SET_CONFIGURATION	= 9,
	USB_REQ_GET_INTERFACE		= 10,
	USB_REQ_SET_INTERFACE		= 11,
	USB_REQ_SYNC_FRAME			= 12,
};

typedef struct usb_dev {
	void* hc;
	char* product;
	char* manufacturer;
	u8 hc_type;
	u8 port;

	u8 addr : 7;
	u8 ls : 1;
	u8 endp : 4;
} usb_dev;

typedef union _attr_packed usb_request {
	struct _attr_packed {
		u8 bmRequestType;
		u8 bRequest;
		u16 wValue;
		u16 wIndex;
		u16 wLength;
	};
	u64 raw;
} usb_request;

enum {
	USB_GET_DESC_DEVICE = 0x0012000001000680,
};

typedef struct _attr_packed usb_DEVICE {
	u8 bLength;
	u8 bDescType;
	u16 bsdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubclass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
} usb_DEVICE;
