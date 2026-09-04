#include <stdint.h>

#include <kernel/donut.h>
#include <kernel/gfx.h>
#include <kernel/platform.h>
#include <kernel/tty.h>

static inline void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void kernel_main(uint32_t multiboot_magic, uintptr_t multiboot_info) {
    kernel_arch_initialize(multiboot_magic, multiboot_info);
    enable_interrupts();

    if (!gfx_has_framebuffer()) {
        terminal_writestring("No framebuffer available, using text mode.\n");
    }

    donut_demo_run();
}
