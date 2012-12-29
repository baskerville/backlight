CC      = gcc
LIBS    = -lm
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -Os
LDFLAGS = -s $(LIBS)

PREFIX    ?= /usr/local
BINPREFIX = $(PREFIX)/bin

SRC = backlight.c
OBJ = $(SRC:.c=.o)

all: options backlight

options:
	@echo "backlight build options:"
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "PREFIX  = $(PREFIX)"

.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

backlight: $(OBJ)
	@echo CC -o $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	@echo "cleaning"
	@rm -f $(OBJ) backlight

install:
	@echo "installing executable files to $(DESTDIR)$(BINPREFIX)"
	@install -D -m 4755 backlight $(DESTDIR)$(BINPREFIX)/backlight

uninstall:
	@echo "removing executable files from $(DESTDIR)$(BINPREFIX)"
	@rm -f $(DESTDIR)$(BINPREFIX)/backlight

.PHONY: all options clean install uninstall
