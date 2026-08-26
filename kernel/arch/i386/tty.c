#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/vga.h>

#include "io.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

void terminal_initialize(void) {
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	terminal_clear();
	terminal_move_to_start();
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_scrolldown() {
	
	size_t y = 0; // current row being overwritten/lost

	// scroll all terminal characters up 1 line - will just discard first line data
  	for (; y < VGA_HEIGHT - 1; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
      		const size_t new_index = y * VGA_WIDTH + x;
			terminal_buffer[new_index] = vga_entry(terminal_buffer[new_index + VGA_WIDTH], terminal_color);
		}
	}
 
  	// fill last line up with spaces
  	for (size_t x = 0; x < VGA_WIDTH; x++) {
    	const size_t index = y * VGA_WIDTH + x;
    	terminal_buffer[index] = vga_entry(' ', terminal_color);
  	}
  
}

void terminal_clear() {
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_move_cursor(size_t x, size_t y) {
    uint16_t pos = y * 80 + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_move_to_start() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_move_cursor(terminal_row, terminal_column);
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

// TO DO: fix escape, caps lock, down key
void terminal_putchar(char c) {
	unsigned char uc = c;

	if (c == '\b') { // backspace
		if (terminal_column == 0) {
			terminal_column = 80;
			if (terminal_row > 0) {
				--terminal_row;
			}
		}
		else {
			--terminal_column;
		}
		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		terminal_move_cursor(terminal_column, terminal_row);
		return;
	}
	else if (c == '\t') { // tab = 4 spaces
		terminal_putchar(' ');
		terminal_putchar(' ');
		terminal_putchar(' ');
		terminal_putchar(' ');
		return;
	}
	else if (c != '\n') {
		terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
	}

	if (++terminal_column == VGA_WIDTH || c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_scrolldown();
			--terminal_row;
		}
	}
	terminal_move_cursor(terminal_column, terminal_row);

}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
