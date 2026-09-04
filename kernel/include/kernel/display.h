#ifndef _KERNEL_DISPLAY_H
#define _KERNEL_DISPLAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct gfx_framebuffer {
	uintptr_t address;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bytes_per_pixel;
	uint8_t framebuffer_type;
	uint8_t red_field_position;
	uint8_t red_mask_size;
	uint8_t green_field_position;
	uint8_t green_mask_size;
	uint8_t blue_field_position;
	uint8_t blue_mask_size;
};

void display_initialize(void);
bool display_bind_framebuffer(const struct gfx_framebuffer* framebuffer);
bool display_is_ready(void);
size_t display_width(void);
size_t display_height(void);
void display_clear(void);
void display_present(void);
void display_putpixel(size_t x, size_t y, uint32_t color);

#endif
