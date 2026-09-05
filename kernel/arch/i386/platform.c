#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/gfx.h>
#include <kernel/multiboot.h>
#include <kernel/platform.h>
#include <kernel/timer.h>

#include "gdt.h"
#include "idt.h"

static bool bind_multiboot_framebuffer(uint32_t multiboot_magic, uintptr_t multiboot_info_ptr) {
	if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC || multiboot_info_ptr == 0U) {
		return false;
	}

	const struct multiboot_info* multiboot_info = (const struct multiboot_info*) multiboot_info_ptr;
	if ((multiboot_info->flags & MULTIBOOT_INFO_FLAG_FRAMEBUFFER) == 0U) {
		return false;
	}

	if (multiboot_info->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
		return false;
	}

	struct gfx_framebuffer framebuffer;
	framebuffer.address = (uintptr_t) multiboot_info->framebuffer_addr;
	framebuffer.width = multiboot_info->framebuffer_width;
	framebuffer.height = multiboot_info->framebuffer_height;
	framebuffer.pitch = multiboot_info->framebuffer_pitch;
	framebuffer.bytes_per_pixel = (uint32_t) ((multiboot_info->framebuffer_bpp + 7U) / 8U);
	framebuffer.framebuffer_type = multiboot_info->framebuffer_type;
	framebuffer.red_field_position = multiboot_info->framebuffer_red_field_position;
	framebuffer.red_mask_size = multiboot_info->framebuffer_red_mask_size;
	framebuffer.green_field_position = multiboot_info->framebuffer_green_field_position;
	framebuffer.green_mask_size = multiboot_info->framebuffer_green_mask_size;
	framebuffer.blue_field_position = multiboot_info->framebuffer_blue_field_position;
	framebuffer.blue_mask_size = multiboot_info->framebuffer_blue_mask_size;

	/* Some boot paths omit RGB masks even though they provide a true-color buffer. */
	if (framebuffer.bytes_per_pixel >= 3U
		&& (framebuffer.red_mask_size == 0U
			|| framebuffer.green_mask_size == 0U
			|| framebuffer.blue_mask_size == 0U)) {
		framebuffer.red_field_position = 16U;
		framebuffer.red_mask_size = 8U;
		framebuffer.green_field_position = 8U;
		framebuffer.green_mask_size = 8U;
		framebuffer.blue_field_position = 0U;
		framebuffer.blue_mask_size = 8U;
	}

	return gfx_bind_framebuffer(&framebuffer);
}

void kernel_arch_initialize(uint32_t multiboot_magic, uintptr_t multiboot_info) {
	gdt_install();
	idt_install();
	timer_install();
	gfx_initialize();
	(void) bind_multiboot_framebuffer(multiboot_magic, multiboot_info);
}
