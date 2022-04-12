CFLAGS := $(CFLAGS) -Wall -Wextra -std=c2x

CFLAGS := $(CFLAGS) $(shell sdl2-config --cflags)
LDFLAGS := $(LDFLAGS) $(shell sdl2-config --libs)

CFLAGS := $(CFLAGS) $(shell pkg-config --cflags -- cairo)
LDFLAGS := $(LDFLAGS) $(shell pkg-config --libs -- cairo)

a.out: fourd.c
	clang $(CFLAGS) $< -o $@ $(LDFLAGS)
