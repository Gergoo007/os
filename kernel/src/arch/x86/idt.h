#pragma once

#include <util/types.h>
#include <util/attrs.h>

#define DECL_ISR(n) _attr_int void exc##n(int_frame* i)

typedef struct idt_entry {
	u16 base0;
	u16 ss;
	u8 ist : 3;
	u8 : 5;
	u8 gate_type : 4;
	u8 : 1;
	u8 dpl : 2;
	u8 present : 1;
	u16 base1;
	u32 base2;
	u32 : 32;
} idt_entry;

typedef struct _attr_packed idtr {
	u16 size; // size-1
	idt_entry* address;
} idtr;

typedef struct cpu_regs {
	u64 r15;
	u64 r14;
	u64 r13;
	u64 r12;
	u64 r11;
	u64 r10;
	u64 r9;
	u64 r8;
	u64 cr2;
	u64 rbp;
	u64 rsi;
	u64 rdi;
	u64 rdx;
	u64 rcx;
	u64 rbx;
	u64 rax;
	u64 exc;
	u64 rflexc;
	u64 err;
	u64 rip;
	u64 cs;
	u64 rfl;
	u64 rsp;
	u64 ss;
} cpu_regs;

typedef struct int_frame int_frame;

DECL_ISR(0);
DECL_ISR(1);
DECL_ISR(2);
DECL_ISR(3);
DECL_ISR(4);
DECL_ISR(5);
DECL_ISR(6);
DECL_ISR(7);
DECL_ISR(8);
DECL_ISR(9);
DECL_ISR(10);
DECL_ISR(11);
DECL_ISR(12);
DECL_ISR(13);
DECL_ISR(14);
DECL_ISR(15);
DECL_ISR(16);
DECL_ISR(17);
DECL_ISR(18);
DECL_ISR(19);

DECL_ISR(64); // KBD
DECL_ISR(65); // PIT
DECL_ISR(66); // HPET
DECL_ISR(67);
DECL_ISR(68);
DECL_ISR(69);
DECL_ISR(70);
DECL_ISR(71);
DECL_ISR(72);

void idt_init();
