/*
 * chip8 -- a CHIP-8 emulator
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>

#include "chip8.h"

// Slurp the contents of a file into a buffer, reading no more than `max` bytes.
size_t slurp(int fd, uint8_t *buf, size_t max)
{
	size_t slurped = 0;

	while (slurped != max) {
		int bytes_read = read(fd, buf + slurped, max - slurped);

		if (bytes_read == 0) {
			// eof
			break;
		}

		slurped += bytes_read;
	}

	return slurped;
}

int main(int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Unable to initialise SDL: %s\n", SDL_GetError());
		return 1;
	}

	if (argc != 2) {
		printf("usage: chip8 <file>\n");
		return 1;
	}

	char *romfile = argv[1];
	printf("Opening ROM: %s\n", romfile);

	int fd = open(romfile, O_RDONLY);

	if (fd == -1) {
		perror("open");
		return 1;
	}

	struct stat file_stats;
	if (fstat(fd, &file_stats) == -1) {
		perror("fstat");
		return 1;
	}

	uint8_t *PRG_ROM = calloc(file_stats.st_size, sizeof(uint8_t));
	size_t program_size = slurp(fd, PRG_ROM, file_stats.st_size);
	printf("PRG_ROM size: %zu\n", program_size);

	struct Machine ctx;
	machine_init(&ctx, PRG_ROM, program_size);
	machine_run(&ctx);

	return 0;
}
