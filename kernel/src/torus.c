#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include <kernel/mesh.h>
#include <kernel/torus.h>

bool torus_build(const struct torus_desc* desc,
	struct mesh_vertex* vertices, size_t vertex_capacity,
	struct mesh_triangle* triangles, size_t triangle_capacity,
	struct mesh* mesh) {
	if (desc == NULL || vertices == NULL || triangles == NULL || mesh == NULL) return false;
	if (desc->major_segments == 0U || desc->minor_segments == 0U) return false;

	const size_t vertex_count = desc->major_segments * desc->minor_segments;
	const size_t triangle_count = vertex_count * 2U;
	if (vertex_count > vertex_capacity || triangle_count > triangle_capacity) return false;

	const float tau = 6.28318530717958647692f;

	for (size_t i = 0; i < vertex_count; ++i) {
		const size_t major = i / desc->minor_segments;
		const size_t minor = i % desc->minor_segments;
		const float u = (float)major / (float)desc->major_segments;
		const float v = (float)minor / (float)desc->minor_segments;
		const float major_angle = u * tau;
		const float minor_angle = v * tau;
		const float major_cos = cosf(major_angle);
		const float major_sin = sinf(major_angle);
		const float minor_cos = cosf(minor_angle);
		const float minor_sin = sinf(minor_angle);
		const float radius = desc->major_radius + desc->tube_radius * minor_cos;

		vertices[i].x = radius * major_cos;
		vertices[i].y = desc->tube_radius * minor_sin;
		vertices[i].z = radius * major_sin;
		vertices[i].nx = minor_cos * major_cos;
		vertices[i].ny = minor_sin;
		vertices[i].nz = minor_cos * major_sin;
		vertices[i].u = u;
		vertices[i].v = v;
	}

	size_t triangle_index = 0;
	for (size_t major = 0; major < desc->major_segments; ++major) {
		for (size_t minor = 0; minor < desc->minor_segments; ++minor) {
			const size_t next_major = (major + 1U) % desc->major_segments;
			const size_t next_minor = (minor + 1U) % desc->minor_segments;
			const size_t a = major * desc->minor_segments + minor;
			const size_t b = next_major * desc->minor_segments + minor;
			const size_t c = next_major * desc->minor_segments + next_minor;
			const size_t d = major * desc->minor_segments + next_minor;

			triangles[triangle_index++] = (struct mesh_triangle){ a, b, c };
			triangles[triangle_index++] = (struct mesh_triangle){ a, c, d };
		}
	}

	mesh->vertices = vertices;
	mesh->vertex_count = vertex_count;
	mesh->triangles = triangles;
	mesh->triangle_count = triangle_count;
	return true;
}
