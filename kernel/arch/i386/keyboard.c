
#include <kernel/gfx.h>
#include <kernel/keyboard.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "io.h"

static bool left_shift_pressed = false;
static bool right_shift_pressed = false;
static bool caps_lock_enabled = false;
static bool extended_sequence = false;

static const char keyboard_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char keyboard_map_shifted[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

enum {
    KEYBOARD_MAP_SIZE = sizeof(keyboard_map) / sizeof(keyboard_map[0])
};

static bool is_alpha_scancode(uint8_t scancode) {
    return scancode < KEYBOARD_MAP_SIZE
        && keyboard_map[scancode] >= 'a'
        && keyboard_map[scancode] <= 'z';
}

void keyboard_handle_irq(void) {
    uint8_t scancode = inb(0x60);

    if (scancode == 0xE0) {
        extended_sequence = true;
        return;
    }

    bool is_break = (scancode & 0x80);
    uint8_t clean_scancode = scancode & 0x7F;

    if (extended_sequence) {
        extended_sequence = false;
        return;
    }

    if (clean_scancode == 0x2A) {
        left_shift_pressed = !is_break;
    } else if (clean_scancode == 0x36) {
        right_shift_pressed = !is_break;
    } else if (clean_scancode == 0x3A) {
        if (!is_break) {
            caps_lock_enabled = !caps_lock_enabled;
        }
    } else if (!is_break) {
        char c = 0;
        bool use_shifted_map = left_shift_pressed || right_shift_pressed;

        if (clean_scancode >= KEYBOARD_MAP_SIZE) {
            return;
        }

        if (is_alpha_scancode(clean_scancode) && caps_lock_enabled) {
            use_shifted_map = !use_shifted_map;
        }

        if (use_shifted_map) {
            c = keyboard_map_shifted[clean_scancode];
        } else {
            c = keyboard_map[clean_scancode];
        }

        if (c != 0) {
            gfx_putc(c);
        }
    }
}
