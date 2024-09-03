#include <userspace/init.h>
#include <mm/vmm.h>
#include <mm/paging.h>

extern void user_teszt();

// void user_teszt() {
// 	asm volatile ("syscall" :: "a"((u32)0));
// 	while (1);
// }

void userspace_init() {
	// 0 - 0x00007fffffffffff újramappelés
	

	// teszt(user_teszt, vmm_alloc());
}
