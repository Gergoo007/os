#include <gfx/console.h>
#include <serial/serial.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/clocks/pit.h>
#include <arch/x86/clocks/hpet.h>
#include <arch/x86/idt.h>
#include <acpi/lai/helpers/sci.h>
#include <acpi/lai/helpers/pm.h>
#include <ps2/kbd.h>
#include <util/stacktrace.h>
#include <userspace/sched/process.h>

#define print_reg(st, reg) printk("%s: %p ", #reg, st->reg)
#define sprint_reg(st, reg) sprintk("%s: %p ", #reg, st->reg)

u8 key_release = 0;

void x86_introutine(cpu_regs* regs) {
	switch (regs->exc) {
		case 0x40: {
			u8 scancode = inb(0x60);

			if (scancode == 0xf0) {
				key_release = 1;
				goto end;
			}
			if (key_release) {
				key_release = 0;
				goto end;
			}

			printk("%c", ps2_kbd_convert(scancode));
end:
			// Send EOI to the LAPIC
			lapic_eoi();
			break;
		}

		case 0x41: {
			pit_tick++;
			// error("pit tick");
			lapic_eoi();
			break;
		}

		case 0x42: {
			// hpet_stop();
			sleep_done = 1;

			lapic_eoi();
			break;
		}

		case 0x43: {
			u16 events = lai_get_sci_event();
			if (events & (1 << 8)) {
				printk("Bekapcsoló gomb benyomva, kikapcs...\n");
				lai_enter_sleep(5);
			}
			lapic_eoi();
			break;
		}

		case 0x44: {
			sched_save_context(regs);
			sched_tick();
			break;
		}

		case 0x0e: {
			// A heapen történt a #PF
			if (regs->cr2 >= 0xffffc00000000000 && regs->cr2 < 0xfffff00000000000) {
				map_page(regs->cr2, (u64)pmm_alloc(), 0b11);
				u64 r = 0;
				asm volatile ("movq %0, %%cr2" :: "r"(r));
				return;
			}

			// A vmm bitmapje
			if (regs->cr2 >= 0xffffb00000000000 && regs->cr2 < 0xffffc00000000000) {
				map_page(regs->cr2, (u64)pmm_alloc(), 0b11);
				u64 r = 0;
				asm volatile ("movq %0, %%cr2" :: "r"(r));
				return;
			}
		}
	}

	if (regs->exc > 32) return;

	sprintk("!! EXC 0x%02x %04x @ %p\n\r", regs->exc, regs->err, regs->rip);
	sprint_reg(regs, rax); sprint_reg(regs, rbx);
	sprintk("\n\r");
	sprint_reg(regs, rcx); sprint_reg(regs, rdx);
	sprintk("\n\r");
	sprint_reg(regs, rdi); sprint_reg(regs, rsi);
	sprintk("\n\r");
	sprint_reg(regs, rdx); sprint_reg(regs, cr2);
	sprintk("\n\r");
	sprint_reg(regs, rip); sprint_reg(regs, rfl);
	sprintk("\n\r");
	sprint_reg(regs, rsp); sprint_reg(regs, rbp);
	sprintk("\n\r");

	printk("!! EXC 0x%02x %04x @ %p\n\r", regs->exc, regs->err, regs->rip);
	print_reg(regs, rax); print_reg(regs, rbx);
	printk("\n\r");
	print_reg(regs, rcx); print_reg(regs, rdx);
	printk("\n\r");
	print_reg(regs, rdi); print_reg(regs, rsi);
	printk("\n\r");
	print_reg(regs, rdx); print_reg(regs, cr2);
	printk("\n\r");
	print_reg(regs, rip); print_reg(regs, rfl);
	printk("\n\r");
	print_reg(regs, rsp); print_reg(regs, rbp);
	printk("\n\r");

	printk("Stacktrace: \n");
	stacktrace(regs->rip, (void*)regs->rbp);

	while (1) asm volatile ("hlt");
}
