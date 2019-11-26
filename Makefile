CFLAGS+=-std=c99 -Wall -Wextra -Werror -Wformat -Wpointer-arith -pedantic-errors -Os
LDLIBS=-lsdl2

INSTALL_PROGRAM=/usr/bin/install
PREFIX=/usr/local

OBJ=chip8.o machine.o

chip8: $(OBJ)

clean:
	$(RM) -v $(OBJ) tree

install: chip8
	$(INSTALL_PROGRAM) -s tree $(DESTDIR)$(PREFIX)/bin/tree

all: chip8
