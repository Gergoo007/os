#include <arch/x86/pic.h>
#include <serial/serial.h>

void pic_init() {
	outb(0xff, 0x21);
	outb(0xff, 0xa1);
}
