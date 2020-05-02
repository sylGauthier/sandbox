DEPS := game

CFLAGS ?= -std=c89 -pedantic -march=native -Wall -g
CFLAGS += -DOPENGEX=1
CFLAGS += $(shell pkg-config --cflags $(DEPS)) -I.
LDFLAGS += $(shell pkg-config --libs $(DEPS)) -lm

OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c src/phys/*.c src/utils/*.c))

.PHONY: all
all: sandbox

sandbox: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) sandbox
