#include <kernel/donut.h>
#include <kernel/gfx.h>
#include <math.h>

extern unsigned int sleep_ms(unsigned int);

enum {
	TEXT_WIDTH = 80,
	TEXT_HEIGHT = 25,
	TEXT_SIZE = TEXT_WIDTH * TEXT_HEIGHT,
	FRAMEBUFFER_MAX_WIDTH = 1024,
	FRAMEBUFFER_MAX_HEIGHT = 768,
	FRAMEBUFFER_MAX_SIZE = FRAMEBUFFER_MAX_WIDTH * FRAMEBUFFER_MAX_HEIGHT,
	PIXEL_BLOCK_SIZE = 2,
	THETA_SAMPLES = 90,
	PHI_SAMPLES = 315
};

static char text_donut_chars[TEXT_SIZE];
static float text_donut_depth[TEXT_SIZE];
static float framebuffer_donut_depth[FRAMEBUFFER_MAX_SIZE];
static float theta_sin[THETA_SAMPLES];
static float theta_cos[THETA_SAMPLES];
static float phi_sin[PHI_SAMPLES];
static float phi_cos[PHI_SAMPLES];
static bool trig_tables_initialized;

static void initialize_trig_tables(void) {
	if (trig_tables_initialized) {
		return;
	}

	for (size_t i = 0; i < THETA_SAMPLES; ++i) {
		const float theta = (float) i * 0.07f;
		theta_sin[i] = sinf(theta);
		theta_cos[i] = cosf(theta);
	}

	for (size_t i = 0; i < PHI_SAMPLES; ++i) {
		const float phi = (float) i * 0.02f;
		phi_sin[i] = sinf(phi);
		phi_cos[i] = cosf(phi);
	}

	trig_tables_initialized = true;
}

static void clear_text_buffers(void) {
	for (size_t i = 0; i < TEXT_SIZE; ++i) {
		text_donut_chars[i] = ' ';
		text_donut_depth[i] = 0.0f;
	}
}

static void clear_framebuffer_depth(size_t width, size_t height) {
	const size_t size = width * height;
	for (size_t i = 0; i < size; ++i) {
		framebuffer_donut_depth[i] = 0.0f;
	}
}

static uint32_t grayscale_color(unsigned int brightness) {
	const uint8_t value = (uint8_t) (brightness > 255U ? 255U : brightness);
	return ((uint32_t) value << 16) | ((uint32_t) value << 8) | (uint32_t) value;
}

static void blit_text_donut_frame(void) {
	const uint8_t color = gfx_text_color(GFX_COLOR_WHITE, GFX_COLOR_BLACK);

	for (size_t y = 0; y < TEXT_HEIGHT; ++y) {
		for (size_t x = 0; x < TEXT_WIDTH; ++x) {
			const size_t index = y * TEXT_WIDTH + x;
			gfx_putcell((unsigned char) text_donut_chars[index], color, x, y);
		}
	}

	gfx_present();
}

static void plot_pixel_block(size_t x, size_t y, uint32_t color, size_t width, size_t height) {
	for (size_t dy = 0; dy < PIXEL_BLOCK_SIZE; ++dy) {
		for (size_t dx = 0; dx < PIXEL_BLOCK_SIZE; ++dx) {
			const size_t px = x + dx;
			const size_t py = y + dy;
			if (px < width && py < height) {
				gfx_putpixel(px, py, color);
			}
		}
	}
}

static void render_text_donut_frame(float A, float B) {
	const float r1 = 1.0f;
	const float r2 = 2.0f;
	const float k2 = 5.0f;
	const float k1_x = 38.0f;
	const float k1_y = 18.0f;
	const char luminance_chars[] = ".,-~:;=!*#$@";

	clear_text_buffers();

	const float sin_a = sinf(A);
	const float cos_a = cosf(A);
	const float sin_b = sinf(B);
	const float cos_b = cosf(B);

	for (size_t theta_index = 0; theta_index < THETA_SAMPLES; ++theta_index) {
		const float sin_theta = theta_sin[theta_index];
		const float cos_theta = theta_cos[theta_index];
		const float circle_x = r2 + r1 * cos_theta;
		const float circle_y = r1 * sin_theta;

		for (size_t phi_index = 0; phi_index < PHI_SAMPLES; ++phi_index) {
			const float sin_phi = phi_sin[phi_index];
			const float cos_phi = phi_cos[phi_index];

			const float x = circle_x * (cos_b * cos_phi + sin_a * sin_b * sin_phi)
				- circle_y * cos_a * sin_b;
			const float y = circle_x * (sin_b * cos_phi - sin_a * cos_b * sin_phi)
				+ circle_y * cos_a * cos_b;
			const float z = k2 + cos_a * circle_x * sin_phi + circle_y * sin_a;
			const float ooz = 1.0f / z;

			const int xp = (int) ((TEXT_WIDTH * 0.5f) + k1_x * ooz * x);
			const int yp = (int) ((TEXT_HEIGHT * 0.5f) - k1_y * ooz * y);

			if (xp < 0 || xp >= TEXT_WIDTH || yp < 0 || yp >= TEXT_HEIGHT) {
				continue;
			}

			const float luminance =
				cos_phi * cos_theta * sin_b
				- cos_a * cos_theta * sin_phi
				- sin_a * sin_theta
				+ cos_b * (cos_a * sin_theta - cos_theta * sin_a * sin_phi);

			const size_t index = (size_t) yp * TEXT_WIDTH + (size_t) xp;
			if (ooz > text_donut_depth[index]) {
				text_donut_depth[index] = ooz;

				const float normalized_luminance =
					(luminance + 1.41421356237f) * (11.0f / 2.82842712474f);
				int luminance_index = (int) normalized_luminance;
				if (luminance_index < 0) {
					luminance_index = 0;
				}
				if (luminance_index > 11) {
					luminance_index = 11;
				}

				text_donut_chars[index] = luminance_chars[luminance_index];
			}
		}
	}

	blit_text_donut_frame();
}

static void render_framebuffer_donut_frame(float A, float B) {
	const size_t framebuffer_width = gfx_width();
	const size_t framebuffer_height = gfx_height();
	const size_t width = framebuffer_width > FRAMEBUFFER_MAX_WIDTH ? FRAMEBUFFER_MAX_WIDTH : framebuffer_width;
	const size_t height = framebuffer_height > FRAMEBUFFER_MAX_HEIGHT ? FRAMEBUFFER_MAX_HEIGHT : framebuffer_height;
	const float r1 = 1.0f;
	const float r2 = 2.0f;
	const float k2 = 5.0f;
	const float k1_x = 38.0f * ((float) width / (float) TEXT_WIDTH);
	const float k1_y = 18.0f * ((float) height / (float) TEXT_HEIGHT);
	const float center_x = (float) width * 0.5f;
	const float center_y = (float) height * 0.5f;

	if (width == 0U || height == 0U) {
		return;
	}

	clear_framebuffer_depth(width, height);
	gfx_clear();

	const float sin_a = sinf(A);
	const float cos_a = cosf(A);
	const float sin_b = sinf(B);
	const float cos_b = cosf(B);

	for (size_t theta_index = 0; theta_index < THETA_SAMPLES; ++theta_index) {
		const float sin_theta = theta_sin[theta_index];
		const float cos_theta = theta_cos[theta_index];
		const float circle_x = r2 + r1 * cos_theta;
		const float circle_y = r1 * sin_theta;

		for (size_t phi_index = 0; phi_index < PHI_SAMPLES; ++phi_index) {
			const float sin_phi = phi_sin[phi_index];
			const float cos_phi = phi_cos[phi_index];

			const float x = circle_x * (cos_b * cos_phi + sin_a * sin_b * sin_phi)
				- circle_y * cos_a * sin_b;
			const float y = circle_x * (sin_b * cos_phi - sin_a * cos_b * sin_phi)
				+ circle_y * cos_a * cos_b;
			const float z = k2 + cos_a * circle_x * sin_phi + circle_y * sin_a;
			const float ooz = 1.0f / z;

			const int xp = (int) (center_x + k1_x * ooz * x);
			const int yp = (int) (center_y - k1_y * ooz * y);

			if (xp < 0 || yp < 0 || (size_t) xp >= width || (size_t) yp >= height) {
				continue;
			}

			const float luminance =
				cos_phi * cos_theta * sin_b
				- cos_a * cos_theta * sin_phi
				- sin_a * sin_theta
				+ cos_b * (cos_a * sin_theta - cos_theta * sin_a * sin_phi);

			const size_t index = (size_t) yp * width + (size_t) xp;
			if (ooz > framebuffer_donut_depth[index]) {
				framebuffer_donut_depth[index] = ooz;

				const float normalized_luminance =
					(luminance + 1.41421356237f) * (255.0f / 2.82842712474f);
				unsigned int brightness = (unsigned int) normalized_luminance;
				if (brightness > 255U) {
					brightness = 255U;
				}

				plot_pixel_block((size_t) xp, (size_t) yp, grayscale_color(brightness), width, height);
			}
		}
	}

	gfx_present();
}

void donut_demo_run(void) {
	float A = 0.0f;
	float B = 0.0f;
	initialize_trig_tables();

	for (;;) {
		if (gfx_has_framebuffer()) {
			render_framebuffer_donut_frame(A, B);
		} else {
			render_text_donut_frame(A, B);
		}

		A += 0.04f;
		B += 0.02f;

		//sleep_ms(4);
	}
}
