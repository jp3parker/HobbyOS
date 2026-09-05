#include <stddef.h>
#include <stdint.h>

#include <math.h>
#include <kernel/gfx.h>
#include <kernel/mesh.h>
#include <kernel/renderer.h>

enum {
	RENDER_WIDTH = 512,
	RENDER_HEIGHT = 384,
	DISPLAY_SCALE = 2
};

struct projected_vertex { int x, y; float inv_z, u_over_z, v_over_z, light_over_z; };

static struct projected_vertex projected_vertices[RENDERER_MAX_VERTICES];
static float depth_buffer[RENDER_WIDTH * RENDER_HEIGHT];

static int edge(int ax, int ay, int bx, int by, int px, int py) {
	return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

static void put_scaled_pixel(int x, int y, uint32_t color) {
	const size_t px = (size_t)x * DISPLAY_SCALE;
	const size_t py = (size_t)y * DISPLAY_SCALE;
	for (int dy = 0; dy < DISPLAY_SCALE; ++dy)
		for (int dx = 0; dx < DISPLAY_SCALE; ++dx)
			gfx_putpixel(px + (size_t)dx, py + (size_t)dy, color);
}

static void rasterize_triangle(const struct projected_vertex* a,
	const struct projected_vertex* b, const struct projected_vertex* c,
	renderer_material_fn material) {
	const int area = edge(a->x, a->y, b->x, b->y, c->x, c->y);
	if (area <= 0) return;

	int min_x = a->x < b->x ? a->x : b->x;
	if (c->x < min_x) min_x = c->x;
	int max_x = a->x > b->x ? a->x : b->x;
	if (c->x > max_x) max_x = c->x;
	int min_y = a->y < b->y ? a->y : b->y;
	if (c->y < min_y) min_y = c->y;
	int max_y = a->y > b->y ? a->y : b->y;
	if (c->y > max_y) max_y = c->y;
	if (min_x < 0) min_x = 0;
	if (min_y < 0) min_y = 0;
	if (max_x >= RENDER_WIDTH) max_x = RENDER_WIDTH - 1;
	if (max_y >= RENDER_HEIGHT) max_y = RENDER_HEIGHT - 1;

	for (int y = min_y; y <= max_y; ++y) {
		for (int x = min_x; x <= max_x; ++x) {
			const int w0 = edge(b->x,b->y,c->x,c->y,x,y);
			const int w1 = edge(c->x,c->y,a->x,a->y,x,y);
			const int w2 = edge(a->x,a->y,b->x,b->y,x,y);
			if (w0 < 0 || w1 < 0 || w2 < 0) continue;
			const float fw0 = (float)w0 / (float)area;
			const float fw1 = (float)w1 / (float)area;
			const float fw2 = (float)w2 / (float)area;
			const float inv_z = fw0*a->inv_z + fw1*b->inv_z + fw2*c->inv_z;
			const size_t index = (size_t)y * RENDER_WIDTH + (size_t)x;
			if (inv_z <= depth_buffer[index]) continue;
			depth_buffer[index] = inv_z;
			const float u = (fw0*a->u_over_z + fw1*b->u_over_z + fw2*c->u_over_z) / inv_z;
			const float v = (fw0*a->v_over_z + fw1*b->v_over_z + fw2*c->v_over_z) / inv_z;
			const float light = (fw0*a->light_over_z + fw1*b->light_over_z + fw2*c->light_over_z) / inv_z;
			put_scaled_pixel(x, y, material(u, v, light));
		}
	}
}

void renderer_initialize(void) {
	for (size_t i = 0; i < RENDER_WIDTH * RENDER_HEIGHT; ++i) depth_buffer[i] = 0.0f;
}

void renderer_render_mesh(const struct mesh* mesh,
	float horizontal_angle, float vertical_angle,
	renderer_material_fn material) {
	if (mesh == NULL || material == NULL) return;
	if (mesh->vertices == NULL || mesh->triangles == NULL) return;
	if (mesh->vertex_count > RENDERER_MAX_VERTICES) return;

	const float sy = sinf(horizontal_angle);
	const float cy = cosf(horizontal_angle);
	const float sx = sinf(vertical_angle);
	const float cx = cosf(vertical_angle);
	const float sz = sinf(horizontal_angle);
	const float cz = cosf(horizontal_angle);

	for (size_t i = 0; i < mesh->vertex_count; ++i) {
		const struct mesh_vertex* vertex = &mesh->vertices[i];
		const float x = vertex->x * cy + vertex->z * sy;
		const float z_rotated = -vertex->x * sy + vertex->z * cy;
		const float y_rotated = vertex->y * cx - z_rotated * sx;
		const float z = vertex->y * sx + z_rotated * cx + 5.0f;
		const float y = y_rotated;
		const float x_rolled = x * cz - y * sz;
		const float y_rolled = x * sz + y * cz;
		const float nx = vertex->nx * cy + vertex->nz * sy;
		const float nz_rotated = -vertex->nx * sy + vertex->nz * cy;
		const float ny = vertex->ny * cx - nz_rotated * sx;
		const float nz = vertex->ny * sx + nz_rotated * cx;
		const float inv_z = 1.0f / z;
		const float diffuse = nx * -0.45f + ny * 0.65f + nz * -0.62f;
		projected_vertices[i].x = (int)(RENDER_WIDTH * 0.5f + x_rolled * 170.0f * inv_z);
		projected_vertices[i].y = (int)(RENDER_HEIGHT * 0.5f - y_rolled * 170.0f * inv_z);
		projected_vertices[i].inv_z = inv_z;
		projected_vertices[i].u_over_z = vertex->u * inv_z;
		projected_vertices[i].v_over_z = vertex->v * inv_z;
		projected_vertices[i].light_over_z = (0.18f + (diffuse > 0.0f ? diffuse : 0.0f)) * inv_z;
	}

	gfx_clear();
	for (size_t i = 0; i < RENDER_WIDTH * RENDER_HEIGHT; ++i) depth_buffer[i] = 0.0f;
	for (size_t i = 0; i < mesh->triangle_count; ++i) {
		const struct mesh_triangle* triangle = &mesh->triangles[i];
		if (triangle->a >= mesh->vertex_count
			|| triangle->b >= mesh->vertex_count
			|| triangle->c >= mesh->vertex_count) {
			continue;
		}
		rasterize_triangle(&projected_vertices[triangle->a],
			&projected_vertices[triangle->b],
			&projected_vertices[triangle->c],
			material);
	}
	gfx_present();
}
