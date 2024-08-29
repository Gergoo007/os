#include <arch/x86/exchandler.h>

#include <gfx/console.h>
#include <serial/serial.h>

#include <mm/paging.h>

#define print_reg(st, reg) printk("%s: %p ", #reg, st->reg)
#define sprint_reg(st, reg) sprintk("%s: %p ", #reg, st->reg)

void x86_onexception(cpu_regs* regs) {
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

	while (1) asm volatile ("hlt");
}
