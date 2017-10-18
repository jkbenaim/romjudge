target  ?= romjudge
objects := $(patsubst %.c,%.o,$(wildcard *.c))
CFLAGS  += -Wall -ggdb -std=c99 -Os

.PHONY: all
all:	$(target)

.PHONY: clean
clean:
	rm -f $(target) $(objects)

$(target): $(objects)
