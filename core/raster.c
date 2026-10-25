#include "raster.h"
#include "tini_math.h"

static uint32_t *g_fb;
static int g_w, g_h;

shape sh_circle(float x, float y, float r) { shape s = {0}; s.kind = SH_CIRCLE; s.x = x; s.y = y; s.a = r; return s; }
shape sh_ellipse(float x, float y, float rx, float ry) { shape s = {0}; s.kind = SH_ELLIPSE; s.x = x; s.y = y; s.a = rx; s.b = ry; return s; }
shape sh_rbox(float x, float y, float hw, float hh, float r) { shape s = {0}; s.kind = SH_RBOX; s.x = x; s.y = y; s.a = hw; s.b = hh; s.r = r; return s; }
shape sh_capsule(float ax, float ay, float bx, float by, float r) { shape s = {0}; s.kind = SH_CAPSULE; s.x = ax; s.y = ay; s.a = bx; s.b = by; s.r = r; return s; }
shape sh_ring(float x, float y, float r, float thick) { shape s = {0}; s.kind = SH_RING; s.x = x; s.y = y; s.a = r; s.b = thick; return s; }
shape sh_clip_above(shape s, float y) { s.clip_bot = y; s.has_bot = 1; return s; }
shape sh_clip_below(shape s, float y) { s.clip_top = y; s.has_top = 1; return s; }
shape sh_rotate(shape s, float rot) { s.rot = rot; return s; }

void rs_begin(uint32_t *fb, int w, int h) { g_fb = fb; g_w = w; g_h = h; }

void rs_clear(uint32_t c) {
    int n = g_w * g_h;
    for (int i = 0; i < n; i++) g_fb[i] = c;
}

void rs_rect(int x0, int y0, int x1, int y1, uint32_t c) {
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > g_w) x1 = g_w;
    if (y1 > g_h) y1 = g_h;
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) g_fb[y * g_w + x] = c;
}

uint32_t rs_mix(uint32_t a, uint32_t b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    int it = (int)(t * 256.0f), ia = 256 - it;
    uint32_t r = (((a & 255) * ia + (b & 255) * it) >> 8);
    uint32_t g = ((((a >> 8) & 255) * ia + ((b >> 8) & 255) * it) >> 8);
    uint32_t bl = ((((a >> 16) & 255) * ia + ((b >> 16) & 255) * it) >> 8);
    return r | (g << 8) | (bl << 16) | 0xFF000000u;
}

static float sdf(const shape *s, float px, float py) {
    float d;
    switch (s->kind) {
    case SH_CIRCLE:
        d = tm_len(px - s->x, py - s->y) - s->a;
        break;
    case SH_ELLIPSE: {
        float k = tm_len((px - s->x) / s->a, (py - s->y) / s->b);
        d = (k - 1.0f) * tm_min(s->a, s->b);
        break;
    }
    case SH_RBOX: {
        float qx = px - s->x, qy = py - s->y;
        if (s->rot != 0.0f) {
            float c = tm_cos(s->rot), sn = tm_sin(s->rot);
            float rx = qx * c + qy * sn, ry = -qx * sn + qy * c;
            qx = rx; qy = ry;
        }
        qx = tm_abs(qx) - (s->a - s->r);
        qy = tm_abs(qy) - (s->b - s->r);
        d = tm_len(tm_max(qx, 0.0f), tm_max(qy, 0.0f)) + tm_min(tm_max(qx, qy), 0.0f) - s->r;
        break;
    }
    case SH_CAPSULE: {
        float pax = px - s->x, pay = py - s->y, bax = s->a - s->x, bay = s->b - s->y;
        float bb = bax * bax + bay * bay;
        float h = bb > 0.0f ? tm_clamp((pax * bax + pay * bay) / bb, 0.0f, 1.0f) : 0.0f;
        d = tm_len(pax - bax * h, pay - bay * h) - s->r;
        break;
    }
    case SH_RING:
    default:
        d = tm_abs(tm_len(px - s->x, py - s->y) - s->a) - s->b;
        break;
    }
    if (s->has_top) d = tm_max(d, s->clip_top - py);
    if (s->has_bot) d = tm_max(d, py - s->clip_bot);
    return d;
}

static void bounds(const shape *s, float grow, int *x0, int *y0, int *x1, int *y1) {
    float l, t, r, b;
    switch (s->kind) {
    case SH_CIRCLE: l = s->x - s->a; r = s->x + s->a; t = s->y - s->a; b = s->y + s->a; break;
    case SH_ELLIPSE: l = s->x - s->a; r = s->x + s->a; t = s->y - s->b; b = s->y + s->b; break;
    case SH_RBOX: {
        float e = s->rot != 0.0f ? tm_len(s->a, s->b) : 0.0f;
        float ex = e > 0.0f ? e : s->a, ey = e > 0.0f ? e : s->b;
        l = s->x - ex; r = s->x + ex; t = s->y - ey; b = s->y + ey;
        break;
    }
    case SH_CAPSULE:
        l = tm_min(s->x, s->a) - s->r; r = tm_max(s->x, s->a) + s->r;
        t = tm_min(s->y, s->b) - s->r; b = tm_max(s->y, s->b) + s->r;
        break;
    case SH_RING:
    default:
        l = s->x - s->a - s->b; r = s->x + s->a + s->b; t = s->y - s->a - s->b; b = s->y + s->a + s->b;
        break;
    }
    if (s->has_top && s->clip_top - grow > t) t = s->clip_top - grow;
    if (s->has_bot && s->clip_bot + grow < b) b = s->clip_bot + grow;
    *x0 = (int)(l - grow - 2.0f); *x1 = (int)(r + grow + 2.0f);
    *y0 = (int)(t - grow - 2.0f); *y1 = (int)(b + grow + 2.0f);
    if (*x0 < 0) *x0 = 0;
    if (*y0 < 0) *y0 = 0;
    if (*x1 > g_w) *x1 = g_w;
    if (*y1 > g_h) *y1 = g_h;
}

void rs_fill_grown(shape s, float grow, uint32_t c) {
    int x0, y0, x1, y1;
    bounds(&s, grow, &x0, &y0, &x1, &y1);
    float alpha = (float)((c >> 24) & 255) / 255.0f;
    for (int y = y0; y < y1; y++) {
        uint32_t *row = g_fb + y * g_w;
        for (int x = x0; x < x1; x++) {
            float d = sdf(&s, (float)x + 0.5f, (float)y + 0.5f) - grow;
            if (d >= 0.5f) continue;
            float cov = tm_clamp(0.5f - d, 0.0f, 1.0f) * alpha;
            row[x] = cov >= 1.0f ? (c | 0xFF000000u) : rs_mix(row[x], c, cov);
        }
    }
}

void rs_fill(shape s, uint32_t c) { rs_fill_grown(s, 0.0f, c); }

void rs_outlined(shape s, float outline, uint32_t fill, uint32_t line) {
    rs_fill_grown(s, outline, line);
    rs_fill(s, fill);
}

void rs_tint(uint32_t c, float amount) {
    if (amount <= 0.0f) return;
    int n = g_w * g_h;
    for (int i = 0; i < n; i++) g_fb[i] = rs_mix(g_fb[i], c, amount);
}
