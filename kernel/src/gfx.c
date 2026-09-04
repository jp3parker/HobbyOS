#include <stddef.h>
#include <stdint.h>

#include <kernel/display.h>
#include <kernel/gfx.h>
#include <kernel/tty.h>
#include <kernel/vga.h>

static const size_t TEXT_WIDTH = 80;
static const size_t TEXT_HEIGHT = 25;

static char shade_from_color(uint32_t color) {
	const unsigned int brightness = (((color >> 16) & 0xFFU) * 30U
		+ ((color >> 8) & 0xFFU) * 59U + (color & 0xFFU) * 11U) / 100U;
	if (brightness < 32U) return ' ';
	if (brightness < 96U) return '.';
	if (brightness < 160U) return ':';
	if (brightness < 224U) return '*';
	return '#';
}

void gfx_initialize(void) {
	display_initialize();
	terminal_initialize();
}

bool gfx_bind_framebuffer(const struct gfx_framebuffer* framebuffer) {
	return display_bind_framebuffer(framebuffer);
}

bool gfx_has_framebuffer(void) {
	return display_is_ready();
}

size_t gfx_width(void) {
	return display_is_ready() ? display_width() : TEXT_WIDTH;
}

size_t gfx_height(void) {
	return display_is_ready() ? display_height() : TEXT_HEIGHT;
}

void gfx_clear(void) {
	if (display_is_ready()) display_clear();
	else terminal_clear();
}

void gfx_present(void) {
	if (display_is_ready()) display_present();
	else terminal_move_to_start();
}

void gfx_putc(char c) {
	if (!display_is_ready()) terminal_putchar(c);
}

void gfx_putcell(unsigned char c, uint8_t color, size_t x, size_t y) {
	if (!display_is_ready()) terminal_putentryat(c, color, x, y);
}

void gfx_putpixel(size_t x, size_t y, uint32_t color) {
	if (display_is_ready()) {
		display_putpixel(x, y, color);
		return;
	}
	terminal_putentryat((unsigned char)shade_from_color(color),
		gfx_text_color(GFX_COLOR_WHITE, GFX_COLOR_BLACK), x, y);
}

uint8_t gfx_text_color(enum gfx_color fg, enum gfx_color bg) {
	return vga_entry_color((enum vga_color)fg, (enum vga_color)bg);
}
