/* TINI core: the whole game, in plain C, shared by every platform.
 *
 * A platform shell (browser, ESP32 device, desktop test tool) only has to:
 *   1. call tini_init() or tini_load() once,
 *   2. call tini_set_clock() and tini_update() every frame,
 *   3. pass taps to tini_tap(),
 *   4. call tini_render() and copy tini_framebuffer() to its screen.
 * The core never touches files, clocks, screens or the network itself.
 */
#ifndef TINI_CORE_API_H
#define TINI_CORE_API_H

#include <stdint.h>

#define TINI_W 368 /* screen size of the Waveshare ESP32-S3-Touch-AMOLED-1.8 */
#define TINI_H 448

#define TINI_SAVE_MAX 64

/* Starts a brand-new random Tini from a seed. */
void tini_init(uint32_t seed);

/* Local time of day in minutes (0..1439). The shell reads its own clock. */
void tini_set_clock(int minute_of_day);

/* Advances the simulation by dt_ms milliseconds of game time. */
void tini_update(uint32_t dt_ms);

/* A tap on the screen, in screen pixels. */
void tini_tap(int x, int y);

/* Draws the current scene into the framebuffer. */
void tini_render(void);

/* TINI_W * TINI_H pixels, bytes in R, G, B, A order. */
uint32_t *tini_framebuffer(void);

/* Writes the save into buf (at least TINI_SAVE_MAX bytes). Returns its length. */
int tini_save(uint8_t *buf, int cap);

/* Restores a save. Returns 1 on success, 0 if the data is not a valid save. */
int tini_load(const uint8_t *buf, int len);

/* A TINI_SAVE_MAX byte buffer owned by the core, for shells that cannot
 * allocate memory themselves (like the browser). */
uint8_t *tini_save_area(void);

/* Short human-readable line about what the Tini is doing, for test panels. */
const char *tini_status(void);

/* The Tini's name. */
const char *tini_name(void);

#endif
