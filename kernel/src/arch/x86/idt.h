#pragma once

#include <util/types.h>
#include <util/attrs.h>

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
	u64 err;
	u64 rip;
	u64 cs;
	u64 rfl;
	u64 rsp;
	u64 ss;
} cpu_regs;

typedef struct int_frame int_frame;
_attr_int void exc0(int_frame* i);
_attr_int void exc1(int_frame* i);
_attr_int void exc2(int_frame* i);
_attr_int void exc3(int_frame* i);
_attr_int void exc4(int_frame* i);
_attr_int void exc5(int_frame* i);
_attr_int void exc6(int_frame* i);
_attr_int void exc7(int_frame* i);
_attr_int void exc8(int_frame* i);
_attr_int void exc9(int_frame* i);
_attr_int void exc10(int_frame* i);
_attr_int void exc11(int_frame* i);
_attr_int void exc12(int_frame* i);
_attr_int void exc13(int_frame* i);
_attr_int void exc14(int_frame* i);
_attr_int void exc15(int_frame* i);
_attr_int void exc16(int_frame* i);
_attr_int void exc17(int_frame* i);
_attr_int void exc18(int_frame* i);
_attr_int void exc19(int_frame* i);

_attr_int void exc64(int_frame* i);

void idt_init();
