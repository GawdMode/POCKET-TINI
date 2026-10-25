/* Drawing: the room and the Tini, in flat colours with bold outlines. */
#include "tini.h"
#include "tini_state.h"
#include "raster.h"

#define INK HEX(0x2B2230) /* the outline colour for everything */
#define LINE 3.0f         /* outline width in pixels */

static const uint32_t SKIN[] = { HEX(0xFFE0C7), HEX(0xF6C9A4), HEX(0xE5AA7E), HEX(0xC98B5E), HEX(0x9A6440), HEX(0x6E4630) };
static const uint32_t HAIR[] = { HEX(0x2E2433), HEX(0x5B3A29), HEX(0xA0632F), HEX(0xE7B75F), HEX(0xE46A4F), HEX(0x8D7BE0), HEX(0x5FB7A6), HEX(0xF29BC0) };
static const uint32_t SHIRT[] = { HEX(0xF4747A), HEX(0x6FB6F0), HEX(0x8FD18A), HEX(0xFFD166), HEX(0xB48CF2), HEX(0xFF9F5A), HEX(0x5FCFC4), HEX(0xF7F3EA) };
static const uint32_t PANTS[] = { HEX(0x3F5C99), HEX(0x4A4458), HEX(0x8A6A4F), HEX(0x2F7A6B), HEX(0x9B4A6A) };

/* ---------- the room ---------- */

static uint32_t sky_colour(int minute) {
    float h = (float)minute / 60.0f;
    uint32_t night = HEX(0x24305E), dawn = HEX(0xF7A88A), day = HEX(0x9FD8F5);
    if (h < 5.5f || h >= 21.0f) return night;
    if (h < 7.0f) return h < 6.25f ? rs_mix(night, dawn, (h - 5.5f) / 0.75f) : rs_mix(dawn, day, (h - 6.25f) / 0.75f);
    if (h < 18.5f) return day;
    if (h < 19.75f) return rs_mix(day, dawn, (h - 18.5f) / 1.25f);
    return rs_mix(dawn, night, (h - 19.75f) / 1.25f);
}

static float night_amount(int minute) {
    float h = (float)minute / 60.0f;
    if (h < 6.0f || h >= 21.0f) return 0.38f;
    if (h < 7.5f) return 0.38f * (7.5f - h) / 1.5f;
    if (h < 19.0f) return 0.0f;
    return 0.38f * (h - 19.0f) / 2.0f;
}

static void draw_room(void) {
    rs_clear(HEX(0xF6E7D2));
    /* wall stripes */
    for (int x = 0; x < TINI_W; x += 46) rs_rect(x, 0, x + 18, 288, HEX(0xF1DEC5));
    /* floor and planks */
    rs_rect(0, 290, TINI_W, TINI_H, HEX(0xDDA874));
    for (int y = 318; y < TINI_H; y += 34) rs_rect(0, y, TINI_W, y + 2, HEX(0xC9915E));
    /* skirting board */
    rs_rect(0, 280, TINI_W, 292, HEX(0xFFF7EC));
    rs_rect(0, 292, TINI_W, 295, INK);
    rs_rect(0, 277, TINI_W, 280, INK);

    /* window with the real sky in it */
    uint32_t sky = sky_colour(W.minute);
    rs_outlined(sh_rbox(100, 120, 62, 56, 10), LINE, HEX(0xFFF7EC), INK);
    rs_outlined(sh_rbox(100, 120, 50, 44, 6), LINE, sky, INK);
    if (tini_is_night(W.minute) || W.minute >= 21 * 60) {
        rs_fill(sh_circle(124, 98, 10), HEX(0xFFF3C4));
        rs_fill(sh_circle(129, 94, 9), sky);
        rs_fill(sh_circle(78, 104, 1.6f), HEX(0xFFF3C4));
        rs_fill(sh_circle(92, 140, 1.4f), HEX(0xFFF3C4));
        rs_fill(sh_circle(132, 138, 1.6f), HEX(0xFFF3C4));
    } else {
        rs_fill(sh_ellipse(84, 108, 18, 7), HEX(0xFFFFFF));
        rs_fill(sh_ellipse(96, 102, 11, 8), HEX(0xFFFFFF));
        rs_fill(sh_ellipse(124, 140, 14, 5), HEX(0xFFFFFF));
    }
    rs_rect(98, 76, 102, 164, INK);
    rs_rect(56, 118, 144, 122, INK);
    rs_outlined(sh_rbox(100, 182, 70, 6, 3), LINE, HEX(0xFFF7EC), INK);

    /* little framed picture */
    rs_outlined(sh_rbox(262, 108, 40, 32, 6), LINE, HEX(0xFFD9A0), INK);
    rs_outlined(sh_circle(262, 112, 12), LINE, HEX(0xF4747A), INK);
    rs_fill(sh_ellipse(262, 134, 34, 6), HEX(0x8FD18A));

    /* rug */
    rs_outlined(sh_ellipse(184, 382, 132, 40), LINE, HEX(0xB9D7F2), INK);
    rs_fill(sh_ellipse(184, 382, 108, 28), HEX(0xA4C7EA));

    /* plant in the corner */
    rs_outlined(sh_rbox(334, 272, 12, 30, 12), LINE, HEX(0x6CC07A), INK);
    rs_outlined(sh_rotate(sh_rbox(320, 262, 9, 24, 9), -0.5f), LINE, HEX(0x5AAE6A), INK);
    rs_outlined(sh_rotate(sh_rbox(348, 260, 9, 24, 9), 0.5f), LINE, HEX(0x7ACC86), INK);
    rs_outlined(sh_rbox(334, 306, 20, 18, 6), LINE, HEX(0xE98B5C), INK);
    rs_fill(sh_rbox(334, 294, 20, 3, 1), HEX(0xD47548));
}

/* ---------- the Tini ---------- */

/* Draws several shapes as one blob: all outlines first, then all fills. */
static void blob(const shape *parts, int n, uint32_t fill) {
    for (int i = 0; i < n; i++) rs_fill_grown(parts[i], LINE, INK);
    for (int i = 0; i < n; i++) rs_fill(parts[i], fill);
}

static void draw_bubble(float cx, float cy) {
    if (W.bubble == BUB_NONE) return;
    float pop = W.bubble_left > 0.0f ? tm_min(1.0f, (W.bubble == BUB_ZZZ ? 1.0f : 1.6f) - W.bubble_left + 0.6f) : 1.0f;
    float r = 18.0f * tm_clamp(pop, 0.6f, 1.0f);
    shape parts[2] = { sh_circle(cx, cy, r), sh_circle(cx - 17, cy + 19, 5) };
    blob(parts, 2, HEX(0xFFFFFF));
    if (W.bubble == BUB_HEART) {
        uint32_t red = HEX(0xF2546B);
        rs_fill(sh_circle(cx - 4.2f, cy - 2.5f, 5.2f), red);
        rs_fill(sh_circle(cx + 4.2f, cy - 2.5f, 5.2f), red);
        rs_fill(sh_rotate(sh_rbox(cx, cy + 1.5f, 5.6f, 5.6f, 1.2f), 0.785f), red);
    } else if (W.bubble == BUB_BANG) {
        rs_fill(sh_capsule(cx, cy - 9, cx, cy + 2, 2.8f), INK);
        rs_fill(sh_circle(cx, cy + 8.5f, 2.8f), INK);
    } else if (W.bubble == BUB_ZZZ) {
        float s = 1.0f;
        rs_fill(sh_capsule(cx - 7 * s, cy - 7 * s, cx + 7 * s, cy - 7 * s, 2.0f), INK);
        rs_fill(sh_capsule(cx + 7 * s, cy - 7 * s, cx - 7 * s, cy + 7 * s, 2.0f), INK);
        rs_fill(sh_capsule(cx - 7 * s, cy + 7 * s, cx + 7 * s, cy + 7 * s, 2.0f), INK);
    }
}

static void draw_tini(float fx, float fy) {
    const tini_look *L = &W.look;
    uint32_t skin = SKIN[L->skin % 6], hair = HAIR[L->hair_color % 8];
    uint32_t shirt = SHIRT[L->shirt % 8], pants = PANTS[L->pants % 5];
    int walking = W.act == ST_WALK, dozing = W.act == ST_DOZE;

    float hop = 0.0f;
    if (W.hop_left > 0.0f) hop = tm_sin((0.5f - W.hop_left) / 0.5f * TM_PI) * 20.0f;
    float bob = walking ? tm_abs(tm_sin(W.anim * 9.0f)) * 3.0f : tm_sin(W.anim * 2.0f) * 1.0f;
    if (dozing) bob = tm_sin(W.anim * 1.2f) * 1.5f;
    float yb = fy - hop - bob;
    float swing = walking ? tm_sin(W.anim * 9.0f) : 0.0f;
    float look = walking ? W.face * 4.0f : 0.0f;

    /* shadow on the floor shrinks while hopping */
    rs_fill(sh_ellipse(fx, fy + 2, 30 - hop * 0.4f, 7 - hop * 0.1f), RGBA(60, 30, 20, 60));

    /* long hair hangs behind everything */
    if (L->hair_style == 1) {
        shape back = sh_rbox(fx, yb - 84, 40, 38, 20);
        rs_outlined(back, LINE, hair, INK);
    }

    /* legs and shoes */
    float lift_l = walking && swing > 0 ? swing * 4.0f : 0.0f;
    float lift_r = walking && swing < 0 ? -swing * 4.0f : 0.0f;
    rs_outlined(sh_capsule(fx - 10, yb - 28, fx - 10, yb - 8 - lift_l, 7), LINE, pants, INK);
    rs_outlined(sh_capsule(fx + 10, yb - 28, fx + 10, yb - 8 - lift_r, 7), LINE, pants, INK);
    rs_outlined(sh_ellipse(fx - 11, yb - 4 - lift_l, 9, 5.5f), LINE, INK, INK);
    rs_outlined(sh_ellipse(fx + 11, yb - 4 - lift_r, 9, 5.5f), LINE, INK, INK);

    /* body */
    rs_outlined(sh_rbox(fx, yb - 42, 23, 20, 12), LINE, shirt, INK);

    /* short arms ending around hip height */
    float hl = swing * 3.0f, hr = -swing * 3.0f;
    if (W.act == ST_HAPPY) { hl = -6.0f; hr = 6.0f; }
    rs_outlined(sh_capsule(fx - 19, yb - 54, fx - 27 + hl, yb - 34, 6.5f), LINE, shirt, INK);
    rs_outlined(sh_capsule(fx + 19, yb - 54, fx + 27 + hr, yb - 34, 6.5f), LINE, shirt, INK);
    rs_outlined(sh_circle(fx - 27 + hl, yb - 31, 6), LINE, skin, INK);
    rs_outlined(sh_circle(fx + 27 + hr, yb - 31, 6), LINE, skin, INK);

    /* big round head */
    float hy = yb - 94;
    rs_outlined(sh_circle(fx, hy, 37), LINE, skin, INK);

    /* hair on top, drawn as one blob so the outline wraps all of it */
    shape hp[4];
    int n = 0;
    hp[n++] = sh_clip_above(sh_ellipse(fx, hy - 6, 40, 34), hy - 10);
    hp[n++] = sh_capsule(fx - 30 + look, hy - 12, fx + 6 + look, hy - 20, 10);
    hp[n++] = sh_capsule(fx + 14 + look, hy - 18, fx + 32, hy - 8, 8);
    if (L->hair_style == 2) hp[n++] = sh_circle(fx, hy - 42, 14);
    blob(hp, n, hair);

    /* face */
    float ex = 13.0f, ey = hy + 6;
    int shut = dozing || W.blink_left > 0.0f;
    for (int side = -1; side <= 1; side += 2) {
        float x = fx + side * ex + look;
        if (shut) {
            rs_fill(sh_clip_below(sh_ring(x, ey - 2, 5, 1.4f), ey - 1), INK);
        } else if (W.act == ST_HAPPY) {
            rs_fill(sh_clip_above(sh_ring(x, ey + 2, 5, 1.6f), ey + 1), INK);
        } else if (L->eyes == 0) {
            rs_fill(sh_ellipse(x, ey, 4.6f, 6.2f), INK);
            rs_fill(sh_circle(x - 1.4f, ey - 2.4f, 1.7f), HEX(0xFFFFFF));
        } else {
            rs_fill(sh_circle(x, ey, 5.4f), INK);
            rs_fill(sh_circle(x - 1.6f, ey - 2.0f, 2.0f), HEX(0xFFFFFF));
            rs_fill(sh_circle(x + 1.8f, ey + 2.0f, 0.9f), HEX(0xFFFFFF));
        }
        if (L->cheeks) rs_fill(sh_ellipse(x + side * 8.0f, ey + 10, 6, 3.6f), RGBA(255, 120, 140, 110));
    }
    float my = hy + 18;
    if (W.act == ST_HAPPY) {
        rs_outlined(sh_clip_below(sh_ellipse(fx + look, my - 1, 6.5f, 6), my - 1), 1.8f, HEX(0xD9475E), INK);
    } else if (dozing) {
        rs_fill(sh_circle(fx + look, my, 2.4f), INK);
    } else {
        rs_fill(sh_clip_below(sh_ring(fx + look, my - 4, 5, 1.4f), my - 3), INK);
    }

    draw_bubble(fx + 46, hy - 50);
}

void tini_draw_scene(uint32_t *fb, float alpha) {
    rs_begin(fb, TINI_W, TINI_H);
    draw_room();
    float t = tm_clamp(alpha, 0.0f, 1.0f);
    draw_tini(tm_lerp(W.px, W.x, t), tm_lerp(W.py, W.y, t));
    rs_tint(HEX(0x1B2340), night_amount(W.minute));
}
