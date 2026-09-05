#ifndef _KERNEL_TORUS_H
#define _KERNEL_TORUS_H

#include <stdbool.h>
#include <stddef.h>

#include <kernel/mesh.h>

struct torus_desc {
	float major_radius;
	float tube_radius;
	size_t major_segments;
	size_t minor_segments;
};

bool torus_build(const struct torus_desc* desc,
	struct mesh_vertex* vertices, size_t vertex_capacity,
	struct mesh_triangle* triangles, size_t triangle_capacity,
	struct mesh* mesh);

#endif
