#ifndef _KERNEL_GFX_H
#define _KERNEL_GFX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum gfx_color {
	GFX_COLOR_BLACK = 0,
	GFX_COLOR_BLUE = 1,
	GFX_COLOR_GREEN = 2,
	GFX_COLOR_CYAN = 3,
	GFX_COLOR_RED = 4,
	GFX_COLOR_MAGENTA = 5,
	GFX_COLOR_BROWN = 6,
	GFX_COLOR_LIGHT_GREY = 7,
	GFX_COLOR_DARK_GREY = 8,
	GFX_COLOR_LIGHT_BLUE = 9,
	GFX_COLOR_LIGHT_GREEN = 10,
	GFX_COLOR_LIGHT_CYAN = 11,
	GFX_COLOR_LIGHT_RED = 12,
	GFX_COLOR_LIGHT_MAGENTA = 13,
	GFX_COLOR_LIGHT_BROWN = 14,
	GFX_COLOR_WHITE = 15,
};

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

void gfx_initialize(void);
bool gfx_bind_framebuffer(const struct gfx_framebuffer* framebuffer);
bool gfx_has_framebuffer(void);
size_t gfx_width(void);
size_t gfx_height(void);
void gfx_clear(void);
void gfx_present(void);
void gfx_putc(char c);
void gfx_putcell(unsigned char c, uint8_t color, size_t x, size_t y);
void gfx_putpixel(size_t x, size_t y, uint32_t color);
uint8_t gfx_text_color(enum gfx_color fg, enum gfx_color bg);

#endif
