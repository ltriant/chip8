/*
 * chip8 -- a CHIP-8 emulator
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <SDL2/SDL.h>

#include "chip8.h"

static int timediff_us(struct timeval *end, struct timeval *start)
{
	int diff = (end->tv_sec - start->tv_sec) * 1000 * 1000 +
		(end->tv_usec - start->tv_usec);

	return diff;
}

// Slurp the contents of a file into a buffer, reading no more than `max` bytes.
static size_t slurp(int fd, uint8_t *buf, size_t max)
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

static void power_up(struct Machine *ctx)
{
	SDL_Window *window = SDL_CreateWindow(
		"chip8",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH * 10,
		SCREEN_HEIGHT * 10,
		SDL_WINDOW_SHOWN
	);

	if (window == NULL) {
		printf("Unable to create window: %s\n", SDL_GetError());
		return;
	}

	SDL_Surface *surface = SDL_GetWindowSurface(window);
	SDL_Event e;
	bool quit = false;

	struct timeval timer_clock_prev;
	gettimeofday(&timer_clock_prev, NULL);

	struct timeval clock_prev;
	gettimeofday(&clock_prev, NULL);

	struct timeval clock_now;

	while (!quit) {
		gettimeofday(&clock_now, NULL);

		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {

			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_1:
					machine_keydown(ctx, 0x1);
					break;
				case SDLK_2:
					machine_keydown(ctx, 0x2);
					break;
				case SDLK_3:
					machine_keydown(ctx, 0x3);
					break;
				case SDLK_4:
					machine_keydown(ctx, 0xc);
					break;
				case SDLK_q:
					machine_keydown(ctx, 0x4);
					break;
				case SDLK_w:
					machine_keydown(ctx, 0x5);
					break;
				case SDLK_e:
					machine_keydown(ctx, 0x6);
					break;
				case SDLK_r:
					machine_keydown(ctx, 0xd);
					break;
				case SDLK_a:
					machine_keydown(ctx, 0x7);
					break;
				case SDLK_s:
					machine_keydown(ctx, 0x8);
					break;
				case SDLK_d:
					machine_keydown(ctx, 0x9);
					break;
				case SDLK_f:
					machine_keydown(ctx, 0xe);
					break;
				case SDLK_z:
					machine_keydown(ctx, 0xa);
					break;
				case SDLK_x:
					machine_keydown(ctx, 0x0);
					break;
				case SDLK_c:
					machine_keydown(ctx, 0xb);
					break;
				case SDLK_v:
					machine_keydown(ctx, 0xf);
					break;
				}

				break;

			case SDL_KEYUP:
				switch (e.key.keysym.sym) {
				case SDLK_1:
					machine_keyup(ctx, 0x1);
					break;
				case SDLK_2:
					machine_keyup(ctx, 0x2);
					break;
				case SDLK_3:
					machine_keyup(ctx, 0x3);
					break;
				case SDLK_4:
					machine_keyup(ctx, 0xc);
					break;
				case SDLK_q:
					machine_keyup(ctx, 0x4);
					break;
				case SDLK_w:
					machine_keyup(ctx, 0x5);
					break;
				case SDLK_e:
					machine_keyup(ctx, 0x6);
					break;
				case SDLK_r:
					machine_keyup(ctx, 0xd);
					break;
				case SDLK_a:
					machine_keyup(ctx, 0x7);
					break;
				case SDLK_s:
					machine_keyup(ctx, 0x8);
					break;
				case SDLK_d:
					machine_keyup(ctx, 0x9);
					break;
				case SDLK_f:
					machine_keyup(ctx, 0xe);
					break;
				case SDLK_z:
					machine_keyup(ctx, 0xa);
					break;
				case SDLK_x:
					machine_keyup(ctx, 0x0);
					break;
				case SDLK_c:
					machine_keyup(ctx, 0xb);
					break;
				case SDLK_v:
					machine_keyup(ctx, 0xf);
					break;
				}

				break;

			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		bool should_render = machine_tick(ctx);

		if (should_render) {
			for (size_t y = 0; y < SCREEN_HEIGHT; y += 1) {
				for (size_t x = 0; x < SCREEN_WIDTH; x += 1) {
					SDL_Rect square = {
						.x = x * 10,
						.y = y * 10,
						.w = 10,
						.h = 10,
					};

					SDL_FillRect(
						surface,
						&square,
						ctx->SCREEN[y][x] ? COLOR_ON : COLOR_OFF
					);
				}
			}

			SDL_UpdateWindowSurface(window);
		}

		int sleep_time = timediff_us(&clock_now, &clock_prev);

		if (sleep_time < US_PER_CLOCK) {
			sleep_time = (US_PER_CLOCK - sleep_time);
			usleep(sleep_time);
		}

		clock_prev = clock_now;

		if (timediff_us(&clock_now, &timer_clock_prev) >= US_PER_TIMER_CLOCK) {
			machine_tick_timers(ctx);
			timer_clock_prev = clock_now;
		}
	}
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
#if DEBUG
	printf("Opening ROM: %s\n", romfile);
#endif

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

#if DEBUG
	printf("PRG_ROM size: %zu\n", program_size);
#endif

	struct Machine machine;
	machine_init(&machine, PRG_ROM, program_size);
	power_up(&machine);

	return 0;
}
