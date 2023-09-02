CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -ggdb -fsanitize=address -std=c11
LIBS=
OBJECTS=lib.o parser.o main.o
OUT=obf.out
ARGS=

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LIBS)

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

.PHONY:
clean:
	rm -rfv $(OUT) $(OBJECTS)

.PHONY: run
run: $(OUT)
	./$^ $(ARGS)
