/* Internal state shared by the simulation and the drawing code. */
#ifndef TINI_STATE_H
#define TINI_STATE_H

#include <stdint.h>
#include "tini_math.h"

#define STEP_MS 50          /* the simulation runs at a fixed 20 steps a second */
#define STEP_S 0.05f

#define FLOOR_TOP 330.0f    /* where feet can stand */
#define FLOOR_BOT 410.0f
#define ROOM_LEFT 64.0f
#define ROOM_RIGHT 296.0f

typedef enum { ST_IDLE, ST_WALK, ST_HAPPY, ST_DOZE } tini_activity;
typedef enum { BUB_NONE, BUB_HEART, BUB_BANG, BUB_ZZZ } bubble_kind;

typedef struct {
    uint8_t skin, hair_color, hair_style, shirt, pants, eyes, cheeks, name;
} tini_look;

typedef struct {
    uint32_t seed;
    tini_look look;
    tm_rng rng;

    float x, y;          /* feet position now */
    float px, py;        /* feet position one step ago, for smooth drawing */
    float tx, ty;        /* walk target */
    float face;          /* -1 looking left, 1 looking right */
    tini_activity act;
    float act_timer;     /* seconds left in the current activity */
    float awake_timer;   /* a tap at night keeps the Tini up this long */

    float anim;          /* seconds, drives bobbing and swinging */
    float blink_timer;
    float blink_left;
    bubble_kind bubble;
    float bubble_left;
    float hop_left;

    uint16_t bond;       /* grows with attention, 0..1000 */
    uint32_t taps;
    int minute;          /* local time of day */
    uint32_t acc_ms;     /* time not yet simulated */
} tini_world;

extern tini_world W;

extern const char *const TINI_NAMES[];
int tini_is_night(int minute);
void tini_draw_scene(uint32_t *fb, float alpha);

#endif
