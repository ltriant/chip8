# chip8

An emulator for the CHIP-8 system, because every emulator developer should probably write one for fun.

![Pong? You mean paddle wars, right?](pong.png)

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
$ ./chip8 roms/pong.ch8
```

# Things To Do

1. Some games still have quirks. Some need to run at different clock rates, and some expect different behaviours. Probably not going to fix this.
