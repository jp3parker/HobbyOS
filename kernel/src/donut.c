#include <kernel/donut.h>
#include <kernel/gfx.h>
#include <math.h>

extern unsigned int sleep_ms(unsigned int);

enum {
	SCREEN_WIDTH = 80,
	SCREEN_HEIGHT = 25,
	SCREEN_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT
};

static char donut_chars[SCREEN_SIZE];
static float donut_depth[SCREEN_SIZE];

static void clear_donut_buffers(void) {
	for (size_t i = 0; i < SCREEN_SIZE; ++i) {
		donut_chars[i] = ' ';
		donut_depth[i] = 0.0f;
	}
}

static void blit_donut_frame(void) {
	const uint8_t color = gfx_text_color(GFX_COLOR_WHITE, GFX_COLOR_BLACK);

	for (size_t y = 0; y < SCREEN_HEIGHT; ++y) {
		for (size_t x = 0; x < SCREEN_WIDTH; ++x) {
			const size_t index = y * SCREEN_WIDTH + x;
			gfx_putcell((unsigned char) donut_chars[index], color, x, y);
		}
	}

	gfx_present();
}

static void render_donut_frame(float A, float B) {
	const float theta_spacing = 0.07f;
	const float phi_spacing = 0.02f;
	const float r1 = 1.0f;
	const float r2 = 2.0f;
	const float k2 = 5.0f;
	const float k1_x = 38.0f;
	const float k1_y = 18.0f;
	const char luminance_chars[] = ".,-~:;=!*#$@";

	clear_donut_buffers();

	const float sin_a = sinf(A);
	const float cos_a = cosf(A);
	const float sin_b = sinf(B);
	const float cos_b = cosf(B);

	for (float theta = 0.0f; theta < 6.28318530717958647692f; theta += theta_spacing) {
		const float sin_theta = sinf(theta);
		const float cos_theta = cosf(theta);
		const float circle_x = r2 + r1 * cos_theta;
		const float circle_y = r1 * sin_theta;

		for (float phi = 0.0f; phi < 6.28318530717958647692f; phi += phi_spacing) {
			const float sin_phi = sinf(phi);
			const float cos_phi = cosf(phi);

			const float x = circle_x * (cos_b * cos_phi + sin_a * sin_b * sin_phi)
				- circle_y * cos_a * sin_b;
			const float y = circle_x * (sin_b * cos_phi - sin_a * cos_b * sin_phi)
				+ circle_y * cos_a * cos_b;
			const float z = k2 + cos_a * circle_x * sin_phi + circle_y * sin_a;
			const float ooz = 1.0f / z;

			const int xp = (int) ((SCREEN_WIDTH * 0.5f) + k1_x * ooz * x);
			const int yp = (int) ((SCREEN_HEIGHT * 0.5f) - k1_y * ooz * y);

			if (xp < 0 || xp >= SCREEN_WIDTH || yp < 0 || yp >= SCREEN_HEIGHT) {
				continue;
			}

			const float luminance =
				cos_phi * cos_theta * sin_b
				- cos_a * cos_theta * sin_phi
				- sin_a * sin_theta
				+ cos_b * (cos_a * sin_theta - cos_theta * sin_a * sin_phi);

			const size_t index = (size_t) yp * SCREEN_WIDTH + (size_t) xp;
			if (ooz > donut_depth[index]) {
				donut_depth[index] = ooz;

				const float normalized_luminance =
					(luminance + 1.41421356237f) * (11.0f / 2.82842712474f);
				int luminance_index = (int) normalized_luminance;
				if (luminance_index < 0) {
					luminance_index = 0;
				}
				if (luminance_index > 11) {
					luminance_index = 11;
				}

				donut_chars[index] = luminance_chars[luminance_index];
			}
		}
	}

	blit_donut_frame();
}

void donut_demo_run(void) {
	float A = 0.0f;
	float B = 0.0f;

	gfx_clear();

	for (;;) {
		render_donut_frame(A, B);

		A += 0.04f;
		B += 0.02f;

		sleep_ms(16);
	}
}
