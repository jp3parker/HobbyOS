#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <stdint.h>

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_clear();
void terminal_move_to_start();
void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y);

#endif
