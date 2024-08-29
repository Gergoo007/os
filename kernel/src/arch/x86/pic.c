#include <arch/x86/pic.h>
#include <serial/serial.h>

void pic_init() {
	outb(0x21, 0xff);
	outb(0xa1, 0xff);
}
