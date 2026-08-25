#include <math.h>

static float wrap_anglef(float x) {
    const float tau = 6.28318530717958647692f;

    while (x >= tau) {
        x -= tau;
    }

    while (x < 0.0f) {
        x += tau;
    }

    return x;
}

static float sinf_impl(float x) {
    const float pi = 3.14159265358979323846f;
    const float half_pi = 1.57079632679489661923f;
    const float tau = 6.28318530717958647692f;

    x = wrap_anglef(x);
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

float sinf(float x) {
    return sinf_impl(x);
}

float cosf(float x) {
    const float half_pi = 1.57079632679489661923f;
    return sinf_impl(x + half_pi);
}

double sin(double x) {
    return (double) sinf((float) x);
}

double cos(double x) {
    return (double) cosf((float) x);
}
