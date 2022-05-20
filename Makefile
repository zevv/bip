
BIN := bip
SRC := main.c osc.c biquad.c sintab.c

OBJS := $(subst .c,.o, $(SRC))
DEPS := $(subst .c,.d, $(SRC))

CC := $(CROSS)gcc

CFLAGS += -I.
CFLAGS += -O3 -g -pedantic -MMD
CFLAGS += -Wall -Werror 

CFLAGS += $(shell pkg-config --cflags sdl2 lua)
LIBS += -lm
LIBS += $(shell pkg-config --libs sdl2 lua)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(BIN) $(OBJS) $(DEPS)

-include $(DEPS)

