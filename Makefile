CFLAGS+=-std=c99 -Wall -Wextra -Werror -Wformat -Wpointer-arith -pedantic-errors -Os
LDLIBS=-lSDL2

INSTALL_PROGRAM=/usr/bin/install
PREFIX=/usr/local

OBJ=chip8.o machine.o

chip8: $(OBJ)

clean:
	$(RM) -v $(OBJ) chip8

install: chip8
	$(INSTALL_PROGRAM) -s chip8 $(DESTDIR)$(PREFIX)/bin/chip8

all: chip8
