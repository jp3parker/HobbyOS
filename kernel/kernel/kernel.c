#include <kernel/tty.h>
#include <kernel/timer.h>
#include <../arch/i386/gdt.h> // TODO: make this kernel_main file arch independent
#include <../arch/i386/idt.h>
#include <../arch/i386/vga.h>

extern unsigned int sleep_ms(unsigned int);

static inline void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void donut_code(void);

static float wrap_angle(float x) {
    const float tau = 6.28318530717958647692f;

    while (x >= tau) {
        x -= tau;
    }

    while (x < 0.0f) {
        x += tau;
    }

    return x;
}

static float approx_sinf(float x) {
    const float pi = 3.14159265358979323846f;
    const float half_pi = 1.57079632679489661923f;
    const float tau = 6.28318530717958647692f;

    x = wrap_angle(x);
    if (x > pi) {
        x -= tau;
    }

    float sign = 1.0f;
    if (x < 0.0f) {
        sign = -1.0f;
        x = -x;
    }

    if (x > half_pi) {
        x = pi - x;
    }

    const float x2 = x * x;
    const float x3 = x2 * x;
    const float x5 = x3 * x2;
    const float x7 = x5 * x2;

    return sign * (x - (x3 / 6.0f) + (x5 / 120.0f) - (x7 / 5040.0f));
}

static float approx_cosf(float x) {
    const float half_pi = 1.57079632679489661923f;
    return approx_sinf(x + half_pi);
}

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
    const uint8_t color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    for (size_t y = 0; y < SCREEN_HEIGHT; ++y) {
        for (size_t x = 0; x < SCREEN_WIDTH; ++x) {
            const size_t index = y * SCREEN_WIDTH + x;
            terminal_putentryat((unsigned char) donut_chars[index], color, x, y);
        }
    }

    terminal_move_to_start();
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

    const float sin_a = approx_sinf(A);
    const float cos_a = approx_cosf(A);
    const float sin_b = approx_sinf(B);
    const float cos_b = approx_cosf(B);

    for (float theta = 0.0f; theta < 6.28318530717958647692f; theta += theta_spacing) {
        const float sin_theta = approx_sinf(theta);
        const float cos_theta = approx_cosf(theta);
        const float circle_x = r2 + r1 * cos_theta;
        const float circle_y = r1 * sin_theta;

        for (float phi = 0.0f; phi < 6.28318530717958647692f; phi += phi_spacing) {
            const float sin_phi = approx_sinf(phi);
            const float cos_phi = approx_cosf(phi);

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

void kernel_main(void) {
    gdt_install();
    idt_install();
    timer_install();
    terminal_initialize();
    enable_interrupts();

    donut_code();
}


void donut_code(void) {
    float A = 0.0f;
    float B = 0.0f;

    terminal_clear();
    terminal_move_to_start();

    for (;;) {
        render_donut_frame(A, B);

        A += 0.04f;
        B += 0.02f;

        sleep_ms(16);
    }
}
