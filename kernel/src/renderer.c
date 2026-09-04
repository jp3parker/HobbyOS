#include <stddef.h>
#include <stdint.h>

#include <math.h>
#include <kernel/gfx.h>
#include <kernel/renderer.h>

enum {
	RENDER_WIDTH = 512,
	RENDER_HEIGHT = 384,
	DISPLAY_SCALE = 2,
	MAJOR_SEGMENTS = 32,
	MINOR_SEGMENTS = 12,
	VERTEX_COUNT = MAJOR_SEGMENTS * MINOR_SEGMENTS,
	TRIANGLE_COUNT = MAJOR_SEGMENTS * MINOR_SEGMENTS * 2,
	TEXTURE_SIZE = 32
};

struct vertex { float x, y, z, nx, ny, nz, u, v; };
struct projected_vertex { int x, y; float inv_z, u_over_z, v_over_z, light; };

static struct vertex torus_vertices[VERTEX_COUNT];
static float depth_buffer[RENDER_WIDTH * RENDER_HEIGHT];

static float sin_fast(float x) {
	const float pi = 3.14159265358979323846f;
	const float half_pi = 1.57079632679489661923f;
	const float tau = 6.28318530717958647692f;
	while (x > pi) x -= tau;
	while (x < -pi) x += tau;
	float sign = 1.0f;
	if (x < 0.0f) {
		sign = -1.0f;
		x = -x;
	}
	if (x > half_pi) x = pi - x;
	const float x2 = x * x;
	return sign * x * (1.0f - x2 / 6.0f + (x2 * x2) / 120.0f - (x2 * x2 * x2) / 5040.0f);
}

static float cos_fast(float x) { return sin_fast(x + 1.57079632679489661923f); }

static int edge(int ax, int ay, int bx, int by, int px, int py) {
	return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

static uint32_t texture_color(int x, int y, float light) {
	const unsigned int base = (((x / 4) ^ (y / 4)) & 1) ? 210U : 95U;
	const unsigned int value = (unsigned int)(base * light);
	const uint8_t channel = (uint8_t)(value > 255U ? 255U : value);
	return ((uint32_t)channel << 16) | ((uint32_t)(channel * 3U / 4U) << 8) | (uint32_t)(channel / 3U);
}

static void put_scaled_pixel(int x, int y, uint32_t color) {
	const size_t px = (size_t)x * DISPLAY_SCALE;
	const size_t py = (size_t)y * DISPLAY_SCALE;
	for (int dy = 0; dy < DISPLAY_SCALE; ++dy)
		for (int dx = 0; dx < DISPLAY_SCALE; ++dx)
			gfx_putpixel(px + (size_t)dx, py + (size_t)dy, color);
}

static void rasterize_triangle(const struct projected_vertex* a,
	const struct projected_vertex* b, const struct projected_vertex* c) {
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
			int tx = (int)(u * (TEXTURE_SIZE - 1));
			int ty = (int)(v * (TEXTURE_SIZE - 1));
			if (tx < 0) tx = 0;
			if (tx >= TEXTURE_SIZE) tx = TEXTURE_SIZE - 1;
			if (ty < 0) ty = 0;
			if (ty >= TEXTURE_SIZE) ty = TEXTURE_SIZE - 1;
			const float light = (a->light + b->light + c->light) / 3.0f;
			put_scaled_pixel(x, y, texture_color(tx, ty, light));
		}
	}
}

void renderer_initialize(void) {
	const float tau = 6.28318530717958647692f;
	const float major_radius = 1.65f;
	const float tube_radius = 0.65f;

	for (size_t i = 0; i < VERTEX_COUNT; ++i) {
		const size_t major = i / MINOR_SEGMENTS;
		const size_t minor = i % MINOR_SEGMENTS;
		const float u = (float)major / (float)MAJOR_SEGMENTS;
		const float v = (float)minor / (float)MINOR_SEGMENTS;
		const float major_angle = u * tau;
		const float minor_angle = v * tau;
		const float major_cos = cos_fast(major_angle);
		const float major_sin = sin_fast(major_angle);
		const float minor_cos = cos_fast(minor_angle);
		const float minor_sin = sin_fast(minor_angle);
		const float radius = major_radius + tube_radius * minor_cos;
		torus_vertices[i].x = radius * major_cos;
		torus_vertices[i].y = tube_radius * minor_sin;
		torus_vertices[i].z = radius * major_sin;
		torus_vertices[i].nx = minor_cos * major_cos;
		torus_vertices[i].ny = minor_sin;
		torus_vertices[i].nz = minor_cos * major_sin;
		torus_vertices[i].u = u;
		torus_vertices[i].v = v;
	}

	for (size_t i = 0; i < RENDER_WIDTH * RENDER_HEIGHT; ++i) depth_buffer[i] = 0.0f;
}

void renderer_render(float horizontal_angle, float vertical_angle) {
	struct projected_vertex projected[VERTEX_COUNT];
	const float sy = sin_fast(horizontal_angle);
	const float cy = cos_fast(horizontal_angle);
	const float sx = sin_fast(vertical_angle);
	const float cx = cos_fast(vertical_angle);

	for (size_t i = 0; i < VERTEX_COUNT; ++i) {
		const struct vertex* vertex = &torus_vertices[i];
		const float x = vertex->x * cy + vertex->z * sy;
		const float z_rotated = -vertex->x * sy + vertex->z * cy;
		const float y = vertex->y * cx - z_rotated * sx;
		const float z = vertex->y * sx + z_rotated * cx + 5.0f;
		const float nx = vertex->nx * cy + vertex->nz * sy;
		const float nz_rotated = -vertex->nx * sy + vertex->nz * cy;
		const float ny = vertex->ny * cx - nz_rotated * sx;
		const float nz = vertex->ny * sx + nz_rotated * cx;
		const float inv_z = 1.0f / z;
		const float diffuse = nx * -0.45f + ny * 0.65f + nz * -0.62f;
		projected[i].x = (int)(RENDER_WIDTH * 0.5f + x * 170.0f * inv_z);
		projected[i].y = (int)(RENDER_HEIGHT * 0.5f - y * 170.0f * inv_z);
		projected[i].inv_z = inv_z;
		projected[i].u_over_z = vertex->u * inv_z;
		projected[i].v_over_z = vertex->v * inv_z;
		projected[i].light = 0.18f + (diffuse > 0.0f ? diffuse : 0.0f);
	}

	gfx_clear();
	for (size_t i = 0; i < RENDER_WIDTH * RENDER_HEIGHT; ++i) depth_buffer[i] = 0.0f;
	for (size_t major = 0; major < MAJOR_SEGMENTS; ++major) {
		for (size_t minor = 0; minor < MINOR_SEGMENTS; ++minor) {
			const size_t next_major = (major + 1) % MAJOR_SEGMENTS;
			const size_t next_minor = (minor + 1) % MINOR_SEGMENTS;
			const size_t a = major * MINOR_SEGMENTS + minor;
			const size_t b = next_major * MINOR_SEGMENTS + minor;
			const size_t c = next_major * MINOR_SEGMENTS + next_minor;
			const size_t d = major * MINOR_SEGMENTS + next_minor;
			rasterize_triangle(&projected[a], &projected[b], &projected[c]);
			rasterize_triangle(&projected[a], &projected[c], &projected[d]);
		}
	}
	gfx_present();
}
