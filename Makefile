DEPS := game

CFLAGS ?= -std=c89 -pedantic -march=native -Wall -O3
CFLAGS += -DOPENGEX=1
CFLAGS += $(shell pkg-config --cflags $(DEPS)) -I.
LDFLAGS += $(shell pkg-config --libs $(DEPS)) -lm

OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c))

.PHONY: all
all: sandbox

sandbox: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) sandbox
