#pragma once

#include <util/types.h>

#include <util/printf.h>
#define sprintk(fmt, ...) printf(sputc, sputs, fmt, ##__VA_ARGS__)

void outb(u8 b, u16 port);
void outw(u16 b, u16 port);
void outl(u32 b, u16 port);

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);

u8 sputc(const char* c);
void sputs(char* str);

static inline void serial_init(u16 port) {
	outb(0x00, port + 1);	// Disable all interrupts
	outb(0x80, port + 3);	// Enable DLAB (set baud rate divisor)
	outb(0x03, port + 0);	// Set divisor to 3 (lo byte) 38400 baud
	outb(0x00, port + 1);	//                  (hi byte)
	outb(0x03, port + 3);	// 8 bits, no parity, one stop bit
	outb(0xC7, port + 2);	// Enable FIFO, clear them, with 14-byte threshold
	outb(0x0B, port + 4);	// IRQs enabled, RTS/DSR set
	outb(0x1E, port + 4);	// Set in loopback mode, test the serial chip
	outb(0xAE, port + 0);	// Test serial chip (send byte 0xAE and check if serial returns same byte)

	// Check if serial is faulty (i.e: not same byte as sent)
	if(inb(port + 0) != 0xAE) {
		asm volatile ("movq %0, %%rax" :: "r"(0xabcdefabcdef));
		while (1);
	}

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb(0x0F, port + 4);
}
