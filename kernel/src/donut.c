#include <stdint.h>

#include <kernel/donut.h>
#include <kernel/mesh.h>
#include <kernel/renderer.h>
#include <kernel/timer.h>
#include <kernel/torus.h>

enum {
	TORUS_MAJOR_SEGMENTS = 64,
	TORUS_MINOR_SEGMENTS = 24,
	TORUS_VERTEX_COUNT = TORUS_MAJOR_SEGMENTS * TORUS_MINOR_SEGMENTS,
	TORUS_TRIANGLE_COUNT = TORUS_MAJOR_SEGMENTS * TORUS_MINOR_SEGMENTS * 2,
};

static struct mesh_vertex torus_vertices[TORUS_VERTEX_COUNT];
static struct mesh_triangle torus_triangles[TORUS_TRIANGLE_COUNT];

static uint32_t donut_material(float u, float v, float light) {
	(void)u;
	(void)v;
	unsigned int value = (unsigned int)(255.0f * light);
	if (value > 255U) value = 255U;
	return ((uint32_t)value << 16) | ((uint32_t)value << 8) | (uint32_t)value;
}

void donut_demo_run(void) {
	float horizontal_angle = 0.0f;
	/* A tilted starting pose makes rotation around the torus' vertical axis visible. */
	float vertical_angle = 0.65f;
	uint32_t next_frame = timer_ticks();
	struct mesh torus;
	const struct torus_desc torus_desc = {
		.major_radius = 2.1f,
		.tube_radius = 1.1f,
		.major_segments = TORUS_MAJOR_SEGMENTS,
		.minor_segments = TORUS_MINOR_SEGMENTS
	};

	if (!torus_build(&torus_desc,
		torus_vertices, TORUS_VERTEX_COUNT,
		torus_triangles, TORUS_TRIANGLE_COUNT,
		&torus)) {
		return;
	}

	renderer_initialize();
	for (;;) {
		renderer_render_mesh(&torus, horizontal_angle, vertical_angle, donut_material);
		horizontal_angle += 0.035f;
		vertical_angle += 0.012f;
		next_frame += 2U;
		timer_wait_until(next_frame);
		if ((int32_t)(next_frame - timer_ticks()) < -2) {
			next_frame = timer_ticks();
		}
	}
}
