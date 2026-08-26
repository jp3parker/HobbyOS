#include <kernel/donut.h>
#include <kernel/gfx.h>
#include <kernel/platform.h>

static inline void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void kernel_main(void) {
    kernel_arch_initialize();
    enable_interrupts();

    donut_demo_run();
}
