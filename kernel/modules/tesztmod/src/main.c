#include <gfx/console.h>

void turiip() {
	printk("Hello world from module++++++!!!!!!!!\n");
    printk("Második string!\n");
}

void mod_main() {
	turiip();
	turiip();
}
