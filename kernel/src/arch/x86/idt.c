#include <arch/x86/idt.h>
#include <mm/vmm.h>
#include <util/mem.h>

idt_entry* idt;

void idt_add_entry(u8 v, u64 isr, u8 type) {
	idt[v].dpl = 0;
	idt[v].gate_type = type;
	idt[v].ist = 0;
	idt[v].base0 =  isr & 0x000000000000ffff;
	idt[v].base1 = (isr & 0x00000000ffff0000) >> 16;
	idt[v].base2 = (isr & 0xffffffff00000000) >> 32;
	idt[v].present = 1;
	idt[v].ss = 0x08;
}

void idt_init() {
	idt = vmm_alloc();
	memset_aligned(idt, 0, 0x1000/8);

	idt_add_entry(0x00, (u64)exc0,  0b1111);
	idt_add_entry(0x01, (u64)exc1,  0b1111);
	idt_add_entry(0x02, (u64)exc2,  0b1111);
	idt_add_entry(0x03, (u64)exc3,  0b1111);
	idt_add_entry(0x04, (u64)exc4,  0b1111);
	idt_add_entry(0x05, (u64)exc5,  0b1111);
	idt_add_entry(0x06, (u64)exc6,  0b1111);
	idt_add_entry(0x07, (u64)exc7,  0b1111);
	idt_add_entry(0x08, (u64)exc8,  0b1111);
	idt_add_entry(0x09, (u64)exc9,  0b1111);
	idt_add_entry(0x0a, (u64)exc10, 0b1111);
	idt_add_entry(0x0b, (u64)exc11, 0b1111);
	idt_add_entry(0x0c, (u64)exc12, 0b1111);
	idt_add_entry(0x0d, (u64)exc13, 0b1111);
	idt_add_entry(0x0e, (u64)exc14, 0b1111);
	idt_add_entry(0x0f, (u64)exc15, 0b1111);
	idt_add_entry(0x10, (u64)exc16, 0b1111);

	idt_add_entry(0x40, (u64)exc64, 0b1110);
	idt_add_entry(0x41, (u64)exc65, 0b1110);
	idt_add_entry(0x42, (u64)exc66, 0b1110);

	asm volatile ("lidt %0" :: "m"((idtr) {
		.address = idt,
		.size = 256*16 - 1,
	}));
}
