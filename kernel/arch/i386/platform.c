#include <kernel/gfx.h>
#include <kernel/platform.h>
#include <kernel/timer.h>

#include "gdt.h"
#include "idt.h"

void kernel_arch_initialize(void) {
	gdt_install();
	idt_install();
	timer_install();
	gfx_initialize();
}
