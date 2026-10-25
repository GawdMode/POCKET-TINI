# Pocket TINI

A cozy little life sim: a randomly generated chibi person lives in a tiny room,
keeps to your clock, and does their own thing while you check in. Think a
modern Tamagotchi, but with a little human.

TINI is built to run in a web browser now and on the Waveshare
ESP32-S3-Touch-AMOLED-1.8 (368 × 448) later, from the same code.

## How it is built

```
core/    the whole game in plain C (no libraries, no files, no clock)
web/     the browser shell: clock in, taps in, pixels out
tests/   headless checks that run the core for simulated days
tools/   build and screenshot helpers
```

The core draws every frame itself into a 368 × 448 pixel buffer using smooth
shapes with bold outlines, so the browser shows exactly what the device screen
will. A platform only has to pass in the time and taps and copy the pixels out
(see `core/tini.h`).

## Build and run

Needs `clang` with the WebAssembly linker (`lld`), `make` and `python3`.

```bash
make test          # headless checks
make web           # builds dist/index.html, one self-contained page
open dist/index.html
```

`make build/snapshot` builds a tool that renders a frame to an image without a
browser.

## Milestones

1. **Hello Tini** (this build): one random Tini in a room that walks around,
   blinks, reacts to taps, follows your clock and dozes at night. Saves in the
   browser. Test speed buttons run the day up to 600 times faster.
2. Meet your Tini: the full random chibi generator and a lineup page.
3. A little life: gradual needs, a real daily rhythm, a rule-based brain.
4. Work and Doots: a job, wages earned by working, a first shop.
5. While you were away: check-in summaries, memories, habits.
6. On the device: the same core on the ESP32-S3 board.

Inspired by [Pocket Tank](https://github.com/mediacutlet/pocket-tank), which
runs a tiny on-device model for its fish on the same board.
