#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/gfx.h>
#include <kernel/tty.h>

#include <kernel/vga.h>

static const size_t TEXT_WIDTH = 80;
static const size_t TEXT_HEIGHT = 25;

static bool framebuffer_enabled = false;
static struct gfx_framebuffer framebuffer;

static uint8_t clamp_u8(unsigned int value) {
	return (uint8_t)(value > 255U ? 255U : value);
}

static char shade_from_color(uint32_t color) {
	const unsigned int red = (color >> 16) & 0xFFU;
	const unsigned int green = (color >> 8) & 0xFFU;
	const unsigned int blue = color & 0xFFU;
	const unsigned int brightness = (red * 30U + green * 59U + blue * 11U) / 100U;

	if (brightness < 32U) {
		return ' ';
	}
	if (brightness < 96U) {
		return '.';
	}
	if (brightness < 160U) {
		return ':';
	}
	if (brightness < 224U) {
		return '*';
	}
	return '#';
}

void gfx_initialize(void) {
	framebuffer_enabled = false;
	terminal_initialize();
}

bool gfx_bind_framebuffer(const struct gfx_framebuffer* fb) {
	if (fb == NULL || fb->address == 0 || fb->width == 0 || fb->height == 0 || fb->bytes_per_pixel == 0) {
		return false;
	}

	framebuffer = *fb;
	framebuffer_enabled = true;
	return true;
}

bool gfx_has_framebuffer(void) {
	return framebuffer_enabled;
}

size_t gfx_width(void) {
	if (framebuffer_enabled) {
		return framebuffer.width;
	}

	return TEXT_WIDTH;
}

size_t gfx_height(void) {
	if (framebuffer_enabled) {
		return framebuffer.height;
	}

	return TEXT_HEIGHT;
}

void gfx_clear(void) {
	if (framebuffer_enabled) {
		uint8_t* base = (uint8_t*) framebuffer.address;
		const size_t row_bytes = (size_t) framebuffer.pitch;

		for (size_t y = 0; y < framebuffer.height; ++y) {
			uint8_t* row = base + y * row_bytes;
			for (size_t i = 0; i < row_bytes; ++i) {
				row[i] = 0;
			}
		}
		return;
	}

	terminal_clear();
}

void gfx_present(void) {
	if (framebuffer_enabled) {
		return;
	}

	terminal_move_to_start();
}

void gfx_putc(char c) {
	if (framebuffer_enabled) {
		return;
	}

	terminal_putchar(c);
}

void gfx_putcell(unsigned char c, uint8_t color, size_t x, size_t y) {
	if (framebuffer_enabled) {
		(void) c;
		(void) color;
		(void) x;
		(void) y;
		return;
	}

	terminal_putentryat(c, color, x, y);
}

void gfx_putpixel(size_t x, size_t y, uint32_t color) {
	if (framebuffer_enabled) {
		uint8_t* base = (uint8_t*) framebuffer.address;
		const size_t offset = y * (size_t) framebuffer.pitch + x * (size_t) framebuffer.bytes_per_pixel;

		if (framebuffer.bytes_per_pixel == 4U) {
			*(uint32_t*) (base + offset) = color;
		} else if (framebuffer.bytes_per_pixel == 3U) {
			uint8_t* pixel = base + offset;
			pixel[0] = (uint8_t)(color & 0xFFU);
			pixel[1] = (uint8_t)((color >> 8) & 0xFFU);
			pixel[2] = (uint8_t)((color >> 16) & 0xFFU);
		} else if (framebuffer.bytes_per_pixel == 1U) {
			const unsigned int brightness =
				(((color >> 16) & 0xFFU) * 30U
				+ ((color >> 8) & 0xFFU) * 59U
				+ (color & 0xFFU) * 11U) / 100U;
			base[offset] = clamp_u8(brightness);
		}
		return;
	}

	terminal_putentryat((unsigned char) shade_from_color(color), gfx_text_color(GFX_COLOR_WHITE, GFX_COLOR_BLACK), x, y);
}

uint8_t gfx_text_color(enum gfx_color fg, enum gfx_color bg) {
	return vga_entry_color((enum vga_color) fg, (enum vga_color) bg);
}
