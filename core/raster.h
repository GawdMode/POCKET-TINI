/* Smooth shape drawing: every shape is a signed distance field, so edges are
 * anti-aliased and any shape can get a bold outline by growing it. */
#ifndef RASTER_H
#define RASTER_H

#include <stdint.h>

#define RGBA(r, g, b, a) ((uint32_t)(r) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 16) | ((uint32_t)(a) << 24))
#define RGB(r, g, b) RGBA(r, g, b, 255)
#define HEX(h) RGB(((h) >> 16) & 255, ((h) >> 8) & 255, (h) & 255)

typedef enum { SH_CIRCLE, SH_ELLIPSE, SH_RBOX, SH_CAPSULE, SH_RING } shape_kind;

typedef struct {
    shape_kind kind;
    float x, y;       /* centre (capsule: point a) */
    float a, b;       /* circle: a = radius; ellipse: radii; rbox: half sizes; capsule: point b; ring: radius, thickness */
    float r;          /* rbox corner radius; capsule radius */
    float rot;        /* rbox rotation in radians */
    float clip_top;   /* keep only y >= clip_top (if has_top) */
    float clip_bot;   /* keep only y <= clip_bot (if has_bot) */
    uint8_t has_top, has_bot;
} shape;

shape sh_circle(float x, float y, float r);
shape sh_ellipse(float x, float y, float rx, float ry);
shape sh_rbox(float x, float y, float hw, float hh, float r);
shape sh_capsule(float ax, float ay, float bx, float by, float r);
shape sh_ring(float x, float y, float r, float thick);
shape sh_clip_above(shape s, float y); /* keep the part above y */
shape sh_clip_below(shape s, float y); /* keep the part below y */
shape sh_rotate(shape s, float rot);

void rs_begin(uint32_t *fb, int w, int h);
void rs_clear(uint32_t c);
void rs_rect(int x0, int y0, int x1, int y1, uint32_t c); /* hard-edged, exclusive x1/y1 */
void rs_fill(shape s, uint32_t c);                       /* filled shape */
void rs_fill_grown(shape s, float grow, uint32_t c);     /* shape grown by `grow` px */
void rs_outlined(shape s, float outline, uint32_t fill, uint32_t line);
void rs_tint(uint32_t c, float amount);                  /* blend whole screen toward c */
uint32_t rs_mix(uint32_t a, uint32_t b, float t);

#endif
