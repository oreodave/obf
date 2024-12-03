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
.PHONY: run
run: $(OUT)
	./$^ $(ARGS)

.PHONY:
clean:
	rm -v *.out *.o *.asm
