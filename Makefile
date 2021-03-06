target  ?= romjudge
objects := $(patsubst %.c,%.o,$(wildcard *.c))
CFLAGS  += -Wall -Og -ggdb -std=c99

.PHONY: all
all:	$(target)

.PHONY: clean
clean:
	rm -f $(target) $(objects)

$(target): $(objects)
