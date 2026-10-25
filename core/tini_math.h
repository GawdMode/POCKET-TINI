/* Small math helpers with no libc dependency, so the core builds freestanding. */
#ifndef TINI_MATH_H
#define TINI_MATH_H

#include <stdint.h>

#define TM_PI 3.14159265f

static inline float tm_sqrt(float x) { return x > 0.0f ? __builtin_sqrtf(x) : 0.0f; }
static inline float tm_abs(float x) { return x < 0.0f ? -x : x; }
static inline float tm_min(float a, float b) { return a < b ? a : b; }
static inline float tm_max(float a, float b) { return a > b ? a : b; }
static inline float tm_clamp(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
static inline float tm_len(float x, float y) { return tm_sqrt(x * x + y * y); }
static inline float tm_lerp(float a, float b, float t) { return a + (b - a) * t; }

/* sin(x) for any x; accurate to about 0.001, plenty for animation. */
static inline float tm_sin(float x) {
    const float tau = 2.0f * TM_PI;
    x = x - tau * (float)(int)(x / tau);
    if (x > TM_PI) x -= tau;
    if (x < -TM_PI) x += tau;
    float y = 1.27323954f * x - 0.405284735f * x * tm_abs(x);
    return 0.225f * (y * tm_abs(y) - y) + y;
}
static inline float tm_cos(float x) { return tm_sin(x + TM_PI * 0.5f); }

/* xorshift32 random numbers: same seed, same Tini, on every platform. */
typedef struct { uint32_t s; } tm_rng;
static inline uint32_t tm_next(tm_rng *r) {
    uint32_t x = r->s ? r->s : 0x9E3779B9u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    r->s = x;
    return x;
}
static inline int tm_range(tm_rng *r, int lo, int hi) { /* inclusive */
    return lo + (int)(tm_next(r) % (uint32_t)(hi - lo + 1));
}
static inline float tm_frand(tm_rng *r) { return (float)(tm_next(r) >> 8) / 16777216.0f; }

#endif
