CC      = gcc
LIBS    = -lm
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -Os
LDFLAGS = -s $(LIBS)

PREFIX    ?= /usr/local
BINPREFIX = $(PREFIX)/bin

SRC = backlight.c
OBJ = $(SRC:.c=.o)

all: backlight

$(OBJ): $(SRC) Makefile

.c.o:
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

backlight: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

install:
	install -D -m 4755 backlight $(DESTDIR)$(BINPREFIX)/backlight

uninstall:
	rm -f $(DESTDIR)$(BINPREFIX)/backlight

clean:
	rm -f $(OBJ) backlight

.PHONY: all clean install uninstall
