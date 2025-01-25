#include <util/mem.h>
#include <serial/serial.h>

void memcpy(void* d, const void* s, size_t count) {
	u8 toalign = count & 7;
	while (toalign) {
		*(u8*)(d++) = *(u8*)(s++);
		toalign--;
	}
	count &= ~(7ULL);

	if (!count) return;

	u64 n = count >> 3;
	asm volatile ("rep movsq"
				: "=D" (d),
				"=S" (s),
				"=c" (n)
				: "0" (d),
				"1" (s),
				"2" (n)
				: "memory");
}
