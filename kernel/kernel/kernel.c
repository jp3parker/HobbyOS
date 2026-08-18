#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <../arch/i386/gdt.h> // TODO: make this kernel_main file arch independent
#include <../arch/i386/idt.h>

static inline void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void kernel_main(void) {
    gdt_install();
    idt_install();

    terminal_initialize();
    printf("Hello, kernel World!\n");
    printf("Interrupts installed.\n");
    printf("Press keys. You should see characters from IRQ1.\n");

    enable_interrupts();

    for (;;) { //sleep until interrupt
        __asm__ volatile ("hlt");
    }
}
