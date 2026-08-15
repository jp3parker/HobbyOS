
#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <stdbool.h>

// helper function to read a byte from a hardware port (assembly wrapper)
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Global state variables to track if shift is held down
static bool shift_pressed = false;

// Table 1: Standard lowercase mapping (Scan Code Set 1)
static const char keyboard_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// Table 2: Uppercase/Symbol mapping when Shift is active
static const char keyboard_map_shifted[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};


void keyboard_poll_loop(void) {
    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);

            // Step 1: Detect if this is a "Make" (Press) or "Break" (Release) code
            bool is_break = (scancode & 0x80);
            
            // Clean the raw scan code (remove the 0x80 bit if it's a break code)
            uint8_t clean_scancode = scancode & 0x7F;

            // Step 2: Handle Shift State changes
            // Scan code 0x2A is Left Shift, 0x36 is Right Shift
            if (clean_scancode == 0x2A || clean_scancode == 0x36) {
                if (is_break) {
                    shift_pressed = false; // User let go of Shift
                } else {
                    shift_pressed = true;  // User is holding down Shift
                }
                continue; // Skip printing anything for the shift key itself
            }

            // Step 3: Print characters (Only on PRESS event, ignore releases)
            if (!is_break) {
                char c = 0;
                
                // Pick the correct table based on our tracked state
                if (shift_pressed) {
                    c = keyboard_map_shifted[clean_scancode];
                } else {
                    c = keyboard_map[clean_scancode];
                }

                if (c != 0) {
                    terminal_putchar(c);
                }
            }
        }
    }
}

// void keyboard_poll_loop(void) {
//     while (1) {
//         // Step 1: Read the Status Port (0x64). 
//         // Bit 0 (0x01) turns to 1 when a key is ready.
//         if (inb(0x64) & 0x01) {
            
//             // Step 2: Read the raw scan code from the Data Port (0x60)
//             uint8_t scancode = inb(0x60);

//             // If the highest bit is set (code >= 0x80), it's a "break code" 
//             // meaning the user just LET GO of the key. We ignore those for now.
//             if (!(scancode & 0x80)) {
                
//                 // Step 3: Turn the hardware number into a readable letter
//                 char c = keyboard_map[scancode];
                
//                 // Step 4: If it's a normal character, print it to the screen!
//                 if (c != 0) {
//                     terminal_putchar(c);
//                 }
//             }
//         }
//     }
// }