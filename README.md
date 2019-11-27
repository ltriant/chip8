# chip8

An emulator for the CHIP-8 system, because every emulator developer should probably write one for fun.

# Building and Running

[SDL2](https://libsdl.org/) is required for the graphics, and it can be installed via many different package managers:

```
$ brew install sdl2
$ sudo apt-get install libsdl2-dev
$ sudo yum install SDL2-devel
```

Or see the [libsdl installation documentation](https://wiki.libsdl.org/Installation) for more options.

After that, it can be built with `make`:

```
$ make all
$ ./chip8 roms/tetris.ch8
```

# Things To Do

1. The sound timer needs to actually make a sound when it hits zero
2. Some games still have quirks
