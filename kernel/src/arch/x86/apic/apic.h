#pragma once

#include <arch/x86/apic/ioapic.h>

#define al _Alignas(0x10)

typedef struct lapic_regs {
	al u32 r0[4];
	al u32 r1[4];
	al u32 lapic_id;
	al u32 version;
	al u32 r2[4];
	al u32 r3[4];
	al u32 r4[4];
	al u32 r5[4];
	al u32 tpr;
	al u32 apr;
	al u32 ppr;
	al u32 eoi;
	al u32 rrd;
	al u32 logical_dest;
	al u32 dest_format;
	al u32 spur_int;
	al u32 in_service0;
	al u32 in_service1;
	al u32 in_service2;
	al u32 in_service3;
	al u32 in_service4;
	al u32 in_service5;
	al u32 in_service6;
	al u32 in_service7;
	al u32 trig_mode0;
	al u32 trig_mode1;
	al u32 trig_mode2;
	al u32 trig_mode3;
	al u32 trig_mode4;
	al u32 trig_mode5;
	al u32 trig_mode6;
	al u32 trig_mode7;
	al u32 irr0;
	al u32 irr1;
	al u32 irr2;
	al u32 irr3;
	al u32 irr4;
	al u32 irr5;
	al u32 irr6;
	al u32 irr7;
	al u32 err_sts;
	al u32 r6[24];
	al u32 lvt_cmci;
	al u32 int_cmd0;
	al u32 int_cmd1;
	al u32 lvt_timer;
	al u32 lvt_thermal;
	al u32 lvt_perf;
	al u32 lvt_lint0;
	al u32 lvt_lint1;
	al u32 lvt_err;
	al u32 timer_inital;
	al u32 timer_current;
	al u32 r7[16];
	al u32 timer_divide;
	al u32 r8[4];
} lapic_regs;

void apic_process_madt(madt* m);
void lapic_eoi();
