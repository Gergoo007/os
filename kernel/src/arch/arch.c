#include <arch/arch.h>
#include <gfx/console.h>
#include <devmgr/devmgr.h>

extern volatile u64 pit_tick;

void sleep(u64 time) {
	if (computer->tmr == TMR_HPET) {
		// TODO: HPET periódus választása amely a 2 hatványa
		// (lehessen shiftelni szorzás helyett)
		// sleep_done = 0;
		// hpet_start(time * 1000000);
		// while (sleep_done == 0) {
		// 	printk("v");
		// }
	} else {
		u64 target = pit_tick + time;
		while (pit_tick < target) {
			asm volatile ("hlt");
		}
	}
}
