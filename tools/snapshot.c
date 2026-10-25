/* Renders one frame of the core to a PPM image, for checking the art
 * without a browser.  usage: snapshot SEED MINUTE MS [tap|walk] OUT.ppm */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/tini.h"

int main(int argc, char **argv) {
    if (argc < 5) { fprintf(stderr, "usage: snapshot SEED MINUTE MS [tap|walk] OUT.ppm\n"); return 2; }
    tini_init((uint32_t)strtoul(argv[1], 0, 10));
    tini_set_clock(atoi(argv[2]));
    const char *mode = argc > 5 ? argv[4] : "";
    const char *out = argv[argc - 1];
    tini_update((uint32_t)atoi(argv[3]));
    if (!strcmp(mode, "tap")) { tini_tap(184, 300); tini_update(250); }
    if (!strcmp(mode, "walk")) { tini_tap(300, 400); tini_update(700); }
    tini_render();
    FILE *f = fopen(out, "wb");
    if (!f) return 1;
    fprintf(f, "P6\n%d %d\n255\n", TINI_W, TINI_H);
    uint32_t *fb = tini_framebuffer();
    for (int i = 0; i < TINI_W * TINI_H; i++) {
        unsigned char px[3] = { (unsigned char)fb[i], (unsigned char)(fb[i] >> 8), (unsigned char)(fb[i] >> 16) };
        fwrite(px, 1, 3, f);
    }
    fclose(f);
    printf("%s\n", tini_status());
    return 0;
}
