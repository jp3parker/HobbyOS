#ifndef _KERNEL_MESH_H
#define _KERNEL_MESH_H

#include <stddef.h>

struct mesh_vertex {
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};

struct mesh_triangle {
	size_t a, b, c;
};

struct mesh {
	const struct mesh_vertex* vertices;
	size_t vertex_count;
	const struct mesh_triangle* triangles;
	size_t triangle_count;
};

#endif
