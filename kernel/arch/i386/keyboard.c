
#include <kernel/keyboard.h>
#include <kernel/tty.h>
#include <stdbool.h>
#include "io.h"

// Global state variable to track if shift is held down
static bool shift_pressed = false;

// Global state variable to track caps lock status
static bool caps_lock_enabled = false;

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
            }
            else if (clean_scancode == 0x3A) {   // Caps Lock
                if (!is_break) {
                    caps_lock_enabled = !caps_lock_enabled;
                }
            }
            else if (!is_break) {
                char c = 0;
                
                // Pick the correct table based on our tracked state
                if (shift_pressed | caps_lock_enabled) {
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
