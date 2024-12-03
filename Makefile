CC=gcc
LIBS=
ARGS=

RELEASE=0
GFLAGS=-Wall -Wextra -Werror -Wswitch-enum -std=c11
DFLAGS=-ggdb -fsanitize=address -fsanitize=undefined -DDEBUG
RFLAGS=-O3
ifeq ($(RELEASE), 1)
CFLAGS=$(GFLAGS) $(RFLAGS)
else
CFLAGS=$(GFLAGS) $(DFLAGS)
endif

.PHONY: all
all: obi.out obc.out

obi.out: lib.c parser.c obi.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

obc.out: lib.c parser.c assembler.c obc.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

OUT=
.PHONY: run clean examples-interpreter examples-compiler
run: $(OUT)
	./$^ $(ARGS)

EXAMPLES=$(wildcard examples/*.bf)

examples: examples-interpreter examples-compiler

examples-interpreter:
	$(foreach x, $(EXAMPLES), $(MAKE) run OUT=obi.out ARGS=$(x);)

examples-compiler:
	$(foreach x, $(EXAMPLES), $(MAKE) run OUT=obc.out ARGS=$(x) && ./a.out;)

clean:
	rm -v *.out *.o *.asm
