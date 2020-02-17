/*
 * chip8 -- a CHIP-8 emulator
 */

#define _GNU_SOURCE

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

SDL_AudioDeviceID playback_device_id;

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

static void beep(void)
{
	float_t tone_volume = 0.1;
	size_t period = 44100 / 440; // A440

	float_t buf[SAMPLES_PER_BEEP] = { 0.0 };

	// Square wave
	for (size_t x = 0; x < SAMPLES_PER_BEEP; x += 1) {
		if ((x / period) % 2 == 0)
			buf[x] = tone_volume;
		else
			buf[x] = -tone_volume;
	}

	int queue_rv = SDL_QueueAudio(
		playback_device_id,
		buf,
		sizeof(buf)
	);

	if (queue_rv != 0)
		PRINT_DEBUG("Unable to queue audio to SDL: %s", SDL_GetError());
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
	bool paused = false;

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
				case SDLK_p:
					paused = ! paused;
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

		if (paused) {
			usleep(5000);
			continue;
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

			// A non-zero sound timer means we beep
			if (ctx->ST > 0)
				beep();
		}
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("usage: chip8 <file>\n");
		return 1;
	}

	char *romfile = argv[1];

	PRINT_DEBUG("Opening ROM: %s\n", romfile);
	int fd = open(romfile, O_RDONLY);

	if (fd == -1) {
		char *error_msg;
		int rv = asprintf(&error_msg, "Unable to open ROM %s", romfile);

		if (rv == -1) {
			perror("open");
		} else {
			perror(error_msg);
			free(error_msg);
		}

		return 1;
	}

	struct stat file_stats;
	if (fstat(fd, &file_stats) == -1) {
		perror("fstat");
		return 1;
	}

	uint8_t *PRG_ROM = calloc(file_stats.st_size, sizeof(uint8_t));
	if (!PRG_ROM) {
		perror("calloc");
		return 1;
	}

	size_t program_size = slurp(fd, PRG_ROM, file_stats.st_size);

	PRINT_DEBUG("PRG_ROM size: %zu\n", program_size);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		printf("Unable to initialise SDL: %s\n", SDL_GetError());
		return 1;
	}

	/*
	 * Initialise the audio subsystem. We're sampling at 44.1kHz, although
	 * it's super overkill for a simple beep.
	 */
	SDL_AudioSpec playback_desired_spec = {
		.freq     = 44100,      // 44.1kHz
		.format   = AUDIO_F32,  // prefer f32's over int16's
		.channels = 2,          // stereo
		.samples  = 1024,
		.callback = NULL,       // enable queued audio
	};

	SDL_AudioSpec playback_spec;

	playback_device_id = SDL_OpenAudioDevice(
		NULL,
		SDL_FALSE,
		&playback_desired_spec,
		&playback_spec,
		SDL_AUDIO_ALLOW_FORMAT_CHANGE
	);

	if (playback_device_id == 0) {
		printf("Unable to initialise audio playback: %s\n", SDL_GetError());
		return 1;
	}

	// Unpause the audio device, otherwise nothing will play...
	SDL_PauseAudioDevice(playback_device_id, SDL_FALSE);

	struct Machine machine;
	machine_init(&machine, PRG_ROM, program_size);
	power_up(&machine);

	return 0;
}
