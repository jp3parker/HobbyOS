#ifndef _KERNEL_RENDERER_H
#define _KERNEL_RENDERER_H

#include <stdint.h>

#include <kernel/mesh.h>

enum {
	RENDERER_MAX_VERTICES = 2048
};

typedef uint32_t (*renderer_material_fn)(float u, float v, float light);

void renderer_initialize(void);
void renderer_render_mesh(const struct mesh* mesh,
	float horizontal_angle, float vertical_angle,
	renderer_material_fn material);

#endif
