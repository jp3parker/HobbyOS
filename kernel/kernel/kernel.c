#include <kernel/donut.h>
#include <kernel/gfx.h>
#include <kernel/timer.h>
#include <../arch/i386/gdt.h> // TODO: make this kernel_main file arch independent
#include <../arch/i386/idt.h>

static inline void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void kernel_main(void) {
    gdt_install();
    idt_install();
    timer_install();
    gfx_initialize();
    enable_interrupts();

    donut_demo_run();
}
