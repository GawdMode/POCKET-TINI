/* The life simulation: what the Tini does, step by step. */
#include "tini.h"
#include "tini_state.h"

tini_world W;
static uint32_t g_fb[TINI_W * TINI_H];
static char g_status[64];
static uint8_t g_save_area[TINI_SAVE_MAX];

uint8_t *tini_save_area(void) { return g_save_area; }

const char *const TINI_NAMES[] = {
    "Momo", "Pip", "Bix", "Nori", "Pogo", "Zuzu", "Lumi", "Taro", "Kiki", "Fen", "Juno", "Olo",
};
#define NAME_COUNT 12

/* The look comes only from the seed, so a Tini never changes on reload. */
static void roll_look(uint32_t seed) {
    tm_rng r = { seed ^ 0xA5A5F00Du };
    for (int i = 0; i < 4; i++) tm_next(&r);
    W.look.skin = (uint8_t)tm_range(&r, 0, 5);
    W.look.hair_color = (uint8_t)tm_range(&r, 0, 7);
    W.look.hair_style = (uint8_t)tm_range(&r, 0, 2);
    W.look.shirt = (uint8_t)tm_range(&r, 0, 7);
    W.look.pants = (uint8_t)tm_range(&r, 0, 4);
    W.look.eyes = (uint8_t)tm_range(&r, 0, 1);
    W.look.cheeks = (uint8_t)tm_range(&r, 0, 1);
    W.look.name = (uint8_t)tm_range(&r, 0, NAME_COUNT - 1);
}

static void reset_body(void) {
    W.rng.s = W.seed * 2654435761u + 1u;
    W.x = W.px = W.tx = (ROOM_LEFT + ROOM_RIGHT) * 0.5f;
    W.y = W.py = W.ty = 370.0f;
    W.face = 1.0f;
    W.act = ST_IDLE;
    W.act_timer = 1.5f;
    W.blink_timer = 2.0f;
    W.bubble = BUB_NONE;
}

void tini_init(uint32_t seed) {
    int minute = W.minute;
    tini_world zero = {0};
    W = zero;
    W.seed = seed ? seed : 1u;
    W.minute = minute;
    roll_look(W.seed);
    reset_body();
}

int tini_is_night(int minute) { return minute >= 22 * 60 || minute < 7 * 60; }

void tini_set_clock(int minute_of_day) {
    if (minute_of_day < 0) minute_of_day = 0;
    W.minute = minute_of_day % 1440;
}

static void say(bubble_kind b, float seconds) { W.bubble = b; W.bubble_left = seconds; }

static void start_idle(float lo, float hi) {
    W.act = ST_IDLE;
    W.act_timer = lo + (hi - lo) * tm_frand(&W.rng);
}

static void walk_to(float x, float y) {
    W.tx = tm_clamp(x, ROOM_LEFT, ROOM_RIGHT);
    W.ty = tm_clamp(y, FLOOR_TOP, FLOOR_BOT);
    W.act = ST_WALK;
}

static void step(void) {
    W.px = W.x; W.py = W.y;
    W.anim += STEP_S;

    if (W.blink_left > 0.0f) W.blink_left -= STEP_S;
    W.blink_timer -= STEP_S;
    if (W.blink_timer <= 0.0f) {
        W.blink_left = 0.12f;
        W.blink_timer = 2.0f + 3.0f * tm_frand(&W.rng);
    }
    if (W.bubble_left > 0.0f) {
        W.bubble_left -= STEP_S;
        if (W.bubble_left <= 0.0f) W.bubble = BUB_NONE;
    }
    if (W.hop_left > 0.0f) W.hop_left -= STEP_S;
    if (W.awake_timer > 0.0f) W.awake_timer -= STEP_S;

    int sleepy = tini_is_night(W.minute) && W.awake_timer <= 0.0f;

    switch (W.act) {
    case ST_IDLE:
        W.act_timer -= STEP_S;
        if (sleepy) { W.act = ST_DOZE; break; }
        if (W.act_timer <= 0.0f) {
            walk_to(ROOM_LEFT + (ROOM_RIGHT - ROOM_LEFT) * tm_frand(&W.rng),
                    FLOOR_TOP + (FLOOR_BOT - FLOOR_TOP) * tm_frand(&W.rng));
        }
        break;
    case ST_WALK: {
        float dx = W.tx - W.x, dy = W.ty - W.y, d = tm_len(dx, dy);
        float speed = 46.0f * STEP_S;
        if (dx > 1.0f) W.face = 1.0f;
        else if (dx < -1.0f) W.face = -1.0f;
        if (d <= speed) {
            W.x = W.tx; W.y = W.ty;
            start_idle(2.0f, 6.0f);
        } else {
            W.x += dx / d * speed;
            W.y += dy / d * speed;
        }
        break;
    }
    case ST_HAPPY:
        W.act_timer -= STEP_S;
        if (W.act_timer <= 0.0f) start_idle(1.0f, 3.0f);
        break;
    case ST_DOZE:
        if (!sleepy) { start_idle(0.5f, 1.5f); break; }
        if (W.bubble == BUB_NONE) say(BUB_ZZZ, 4.0f);
        break;
    }
}

void tini_update(uint32_t dt_ms) {
    W.acc_ms += dt_ms;
    int steps = 0;
    while (W.acc_ms >= STEP_MS && steps < 4000) {
        step();
        W.acc_ms -= STEP_MS;
        steps++;
    }
    if (W.acc_ms >= STEP_MS) W.acc_ms = 0; /* very long gaps are skipped, not replayed */
}

static int hits_tini(float x, float y) {
    return tm_abs(x - W.x) < 44.0f && y > W.y - 140.0f && y < W.y + 8.0f;
}

void tini_tap(int sx, int sy) {
    float x = (float)sx, y = (float)sy;
    if (W.act == ST_DOZE) W.awake_timer = 30.0f;
    if (hits_tini(x, y)) {
        W.act = ST_HAPPY;
        W.act_timer = 1.4f;
        W.hop_left = 0.5f;
        if (W.bond < 1000) W.bond++;
        W.taps++;
        say(BUB_HEART, 1.6f);
    } else if (y >= FLOOR_TOP - 20.0f) {
        walk_to(x, y);
        say(BUB_BANG, 0.9f);
    } else if (W.act == ST_DOZE) {
        start_idle(1.0f, 2.0f);
        say(BUB_BANG, 0.9f);
    }
}

void tini_render(void) {
    tini_draw_scene(g_fb, (float)W.acc_ms / (float)STEP_MS);
}

uint32_t *tini_framebuffer(void) { return g_fb; }

const char *tini_name(void) { return TINI_NAMES[W.look.name % NAME_COUNT]; }

static void cat(char **p, const char *s, char *end) {
    while (*s && *p < end) *(*p)++ = *s++;
}

const char *tini_status(void) {
    const char *what = "is standing around";
    switch (W.act) {
    case ST_WALK: what = W.bubble == BUB_BANG ? "is heading over" : "is wandering"; break;
    case ST_HAPPY: what = "loves the attention"; break;
    case ST_DOZE: what = "is dozing"; break;
    default: break;
    }
    char *p = g_status, *end = g_status + sizeof(g_status) - 1;
    cat(&p, tini_name(), end);
    cat(&p, " ", end);
    cat(&p, what, end);
    *p = 0;
    return g_status;
}

/* Save layout, version 1 (little endian):
 *   0  'T' 'I' 'N' 'I'
 *   4  version (1)
 *   5  seed (u32)
 *   9  bond (u16)
 *  11  taps (u32)
 *  15  checksum (sum of bytes 0..14, u8)
 * New versions append fields; tini_load keeps reading old ones. */
#define SAVE_VERSION 1
#define SAVE_LEN_V1 16

static void put32(uint8_t *b, uint32_t v) { b[0] = (uint8_t)v; b[1] = (uint8_t)(v >> 8); b[2] = (uint8_t)(v >> 16); b[3] = (uint8_t)(v >> 24); }
static uint32_t get32(const uint8_t *b) { return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24); }

int tini_save(uint8_t *b, int cap) {
    if (cap < SAVE_LEN_V1) return 0;
    b[0] = 'T'; b[1] = 'I'; b[2] = 'N'; b[3] = 'I';
    b[4] = SAVE_VERSION;
    put32(b + 5, W.seed);
    b[9] = (uint8_t)W.bond; b[10] = (uint8_t)(W.bond >> 8);
    put32(b + 11, W.taps);
    uint8_t sum = 0;
    for (int i = 0; i < 15; i++) sum = (uint8_t)(sum + b[i]);
    b[15] = sum;
    return SAVE_LEN_V1;
}

int tini_load(const uint8_t *b, int len) {
    if (len < SAVE_LEN_V1 || b[0] != 'T' || b[1] != 'I' || b[2] != 'N' || b[3] != 'I') return 0;
    if (b[4] < 1 || b[4] > SAVE_VERSION) return 0;
    uint8_t sum = 0;
    for (int i = 0; i < 15; i++) sum = (uint8_t)(sum + b[i]);
    if (sum != b[15]) return 0;
    tini_init(get32(b + 5));
    W.bond = (uint16_t)(b[9] | (b[10] << 8));
    if (W.bond > 1000) W.bond = 1000;
    W.taps = get32(b + 11);
    return 1;
}
