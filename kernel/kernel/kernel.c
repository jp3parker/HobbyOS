#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <../arch/i386/gdt.h> // TODO: make this kernel_main file arch independent

void kernel_main(void) {

	gdt_install();

	terminal_initialize();
	printf("Hello, kernel World!\n");

	printf("Keyboard polling active. Start typing:\n\n");

	// Handing over control to the infinite keyboard loop
    keyboard_poll_loop();
}
