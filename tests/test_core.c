/* Headless checks for the core. Run with: make test */
#include <stdio.h>
#include <string.h>
#include "../core/tini.h"
#include "../core/tini_state.h"

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); fails++; } } while (0)

static void test_same_seed_same_tini(void) {
    tini_init(42);
    tini_look a = W.look;
    char name[16];
    strcpy(name, tini_name());
    tini_init(42);
    CHECK(memcmp(&a, &W.look, sizeof a) == 0, "same seed gives the same look");
    CHECK(strcmp(name, tini_name()) == 0, "same seed gives the same name");
}

static void test_seeds_vary(void) {
    int differ = 0;
    tini_init(1);
    tini_look first = W.look;
    for (uint32_t s = 2; s < 40; s++) {
        tini_init(s);
        if (memcmp(&first, &W.look, sizeof first) != 0) differ++;
    }
    CHECK(differ > 30, "different seeds give different Tinis");
}

static void test_a_day_stays_in_the_room(void) {
    tini_init(7);
    int walked = 0, dozed = 0;
    for (int minute = 0; minute < 1440; minute++) {
        tini_set_clock(minute);
        for (int i = 0; i < 20; i++) {
            tini_update(1000);
            if (W.act == ST_WALK) walked = 1;
            if (W.act == ST_DOZE) dozed = 1;
            CHECK(W.x >= ROOM_LEFT && W.x <= ROOM_RIGHT, "x stays in the room");
            CHECK(W.y >= FLOOR_TOP - 1 && W.y <= FLOOR_BOT + 1, "y stays on the floor");
            if (fails > 5) return;
        }
    }
    CHECK(walked, "the Tini walks during the day");
    CHECK(dozed, "the Tini dozes at night");
}

static void test_taps(void) {
    tini_init(9);
    tini_set_clock(12 * 60);
    tini_update(100);
    int before = W.bond;
    tini_tap((int)W.x, (int)W.y - 90);
    CHECK(W.act == ST_HAPPY, "tapping the Tini makes it happy");
    CHECK(W.bond == before + 1, "tapping the Tini raises bond");
    tini_update(3000);
    tini_tap(300, 400);
    CHECK(W.act == ST_WALK, "tapping the floor sends the Tini there");
    for (int i = 0; i < 200 && W.act == ST_WALK; i++) tini_update(100);
    CHECK(W.x > 290 && W.y > 395, "the Tini arrives where you tapped");
}

static void test_save_round_trip(void) {
    uint8_t buf[TINI_SAVE_MAX];
    tini_init(1234);
    for (int i = 0; i < 5; i++) { tini_tap((int)W.x, (int)W.y - 90); tini_update(2000); }
    tini_look look = W.look;
    int bond = W.bond;
    int n = tini_save(buf, sizeof buf);
    CHECK(n > 0, "save writes bytes");
    tini_init(99);
    CHECK(tini_load(buf, n) == 1, "a good save loads");
    CHECK(memcmp(&look, &W.look, sizeof look) == 0, "the look survives a reload");
    CHECK(W.bond == bond, "bond survives a reload");
    buf[6] ^= 0x40;
    CHECK(tini_load(buf, n) == 0, "a damaged save is refused");
    CHECK(tini_load((const uint8_t *)"nope", 4) == 0, "garbage is refused");
}

static void test_long_absence_is_safe(void) {
    tini_init(5);
    tini_update(7u * 24u * 3600u * 1000u);
    CHECK(W.acc_ms < STEP_MS, "a week away does not leave time piled up");
    tini_render();
    CHECK(tini_framebuffer()[0] != 0, "rendering works after a long absence");
}

int main(void) {
    test_same_seed_same_tini();
    test_seeds_vary();
    test_a_day_stays_in_the_room();
    test_taps();
    test_save_round_trip();
    test_long_absence_is_safe();
    if (fails) { printf("%d check(s) failed\n", fails); return 1; }
    printf("all core checks passed\n");
    return 0;
}
