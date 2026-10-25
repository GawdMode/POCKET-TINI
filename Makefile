# TINI build. `make` builds and tests everything; `make web` makes the browser page.
CC ?= cc
CFLAGS = -O2 -Wall -Wextra -std=c99
CORE = core/sim.c core/draw.c core/raster.c core/freestanding.c
EXPORTS = tini_init tini_set_clock tini_update tini_tap tini_render tini_framebuffer \
          tini_save tini_load tini_save_area tini_status tini_name
WASM_FLAGS = --target=wasm32 -nostdlib -ffreestanding -DTINI_NO_LIBC -O2 -Wall -Wextra -std=c99 \
             -Wl,--no-entry -Wl,--strip-all $(addprefix -Wl$(comma)--export=,$(EXPORTS))
comma := ,

all: test web

build:
	mkdir -p build

build/test_core: tests/test_core.c $(CORE) core/*.h | build
	$(CC) $(CFLAGS) -o $@ tests/test_core.c $(CORE) -lm

build/snapshot: tools/snapshot.c $(CORE) core/*.h | build
	$(CC) $(CFLAGS) -o $@ tools/snapshot.c $(CORE) -lm

build/tini.wasm: $(CORE) core/*.h | build
	clang $(WASM_FLAGS) -o $@ $(CORE)

test: build/test_core
	./build/test_core

web: build/tini.wasm web/shell.html web/shell.js
	python3 tools/build_web.py build/tini.wasm dist/index.html

clean:
	rm -rf build dist

.PHONY: all test web clean
