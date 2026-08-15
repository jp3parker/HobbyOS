#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/keyboard.h>

void kernel_main(void) {
	terminal_initialize();
	printf("Hello, kernel World!\n");

	printf("Keyboard polling active. Start typing:\n\n");

	// Handing over control to the infinite keyboard loop
    keyboard_poll_loop();
}
