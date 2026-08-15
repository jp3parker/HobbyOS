
#include <kernel/keyboard.h>
#include <kernel/tty.h>

// helper function to read a byte from a hardware port (assembly wrapper)
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// map to read hardware codes and map them to ascii characters
static const char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

void keyboard_poll_loop(void) {
    while (1) {
        // Step 1: Read the Status Port (0x64). 
        // Bit 0 (0x01) turns to 1 when a key is ready.
        if (inb(0x64) & 0x01) {
            
            // Step 2: Read the raw scan code from the Data Port (0x60)
            uint8_t scancode = inb(0x60);

            // If the highest bit is set (code >= 0x80), it's a "break code" 
            // meaning the user just LET GO of the key. We ignore those for now.
            if (!(scancode & 0x80)) {
                
                // Step 3: Turn the hardware number into a readable letter
                char c = keyboard_map[scancode];
                
                // Step 4: If it's a normal character, print it to the screen!
                if (c != 0) {
                    terminal_putchar(c);
                }
            }
        }
    }
}