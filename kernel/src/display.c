#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/display.h>

enum {
	DISPLAY_MAX_WIDTH = 1024,
	DISPLAY_MAX_HEIGHT = 768,
	DISPLAY_MAX_BYTES_PER_PIXEL = 4,
	DISPLAY_MAX_PITCH = DISPLAY_MAX_WIDTH * DISPLAY_MAX_BYTES_PER_PIXEL
};

static bool display_ready;
static struct gfx_framebuffer framebuffer;
static uint8_t backbuffer[DISPLAY_MAX_HEIGHT * DISPLAY_MAX_PITCH];

static uint8_t clamp_u8(unsigned int value) {
	return (uint8_t)(value > 255U ? 255U : value);
}

static uint32_t scale_component(uint8_t component, uint8_t mask_size) {
	if (mask_size == 0U) return 0U;
	const uint32_t max_value = mask_size >= 31U ? 0xFFFFFFFFU : ((1U << mask_size) - 1U);
	return (uint32_t)((component * max_value) / 255U);
}

static uint32_t pack_color(uint32_t color) {
	uint32_t packed = 0U;
	packed |= scale_component((uint8_t)((color >> 16) & 0xFFU), framebuffer.red_mask_size) << framebuffer.red_field_position;
	packed |= scale_component((uint8_t)((color >> 8) & 0xFFU), framebuffer.green_mask_size) << framebuffer.green_field_position;
	packed |= scale_component((uint8_t)(color & 0xFFU), framebuffer.blue_mask_size) << framebuffer.blue_field_position;
	return packed;
}

void display_initialize(void) {
	display_ready = false;
}

bool display_bind_framebuffer(const struct gfx_framebuffer* candidate) {
	if (candidate == NULL || candidate->address == 0U || candidate->width == 0U || candidate->height == 0U) return false;
	if (candidate->bytes_per_pixel == 0U || candidate->bytes_per_pixel > DISPLAY_MAX_BYTES_PER_PIXEL) return false;
	if (candidate->framebuffer_type != 1U) return false;
	if (candidate->width > DISPLAY_MAX_WIDTH || candidate->height > DISPLAY_MAX_HEIGHT) return false;
	if (candidate->pitch > DISPLAY_MAX_PITCH || candidate->pitch < candidate->width * candidate->bytes_per_pixel) return false;

	framebuffer = *candidate;
	display_ready = true;
	return true;
}

bool display_is_ready(void) { return display_ready; }

size_t display_width(void) { return display_ready ? framebuffer.width : 0U; }
size_t display_height(void) { return display_ready ? framebuffer.height : 0U; }

void display_clear(void) {
	if (!display_ready) return;
	for (size_t y = 0; y < framebuffer.height; ++y) {
		memset(backbuffer + y * framebuffer.pitch, 0, framebuffer.pitch);
	}
}

void display_present(void) {
	if (!display_ready) return;
	for (size_t y = 0; y < framebuffer.height; ++y) {
		memcpy((uint8_t*)framebuffer.address + y * framebuffer.pitch,
			backbuffer + y * framebuffer.pitch, framebuffer.pitch);
	}
}

void display_putpixel(size_t x, size_t y, uint32_t color) {
	if (!display_ready || x >= framebuffer.width || y >= framebuffer.height) return;
	const size_t offset = y * framebuffer.pitch + x * framebuffer.bytes_per_pixel;
	uint8_t* pixel = backbuffer + offset;
	const uint32_t packed = pack_color(color);
	if (framebuffer.bytes_per_pixel == 4U) {
		memcpy(pixel, &packed, sizeof(packed));
	} else if (framebuffer.bytes_per_pixel == 3U) {
		pixel[0] = (uint8_t)packed; pixel[1] = (uint8_t)(packed >> 8); pixel[2] = (uint8_t)(packed >> 16);
	} else if (framebuffer.bytes_per_pixel == 2U) {
		pixel[0] = (uint8_t)packed; pixel[1] = (uint8_t)(packed >> 8);
	} else {
		const unsigned int brightness = (((color >> 16) & 0xFFU) * 30U + ((color >> 8) & 0xFFU) * 59U + (color & 0xFFU) * 11U) / 100U;
		*pixel = clamp_u8(brightness);
	}
}
