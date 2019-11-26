#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "chip8.h"

void machine_run(struct Machine *ctx)
{
	printf("Starting the machine\n");

	SDL_Window *window = SDL_CreateWindow(
		"chip8",
		100,
		100,
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

	uint8_t op1;
	uint8_t op2;
	uint16_t opcode;
	uint16_t nnn;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {

			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_1:
					ctx->KEYDOWN[0x1] = true;
					break;
				case SDLK_2:
					ctx->KEYDOWN[0x2] = true;
					break;
				case SDLK_3:
					ctx->KEYDOWN[0x3] = true;
					break;
				case SDLK_4:
					ctx->KEYDOWN[0xc] = true;
					break;
				case SDLK_q:
					ctx->KEYDOWN[0x4] = true;
					break;
				case SDLK_w:
					ctx->KEYDOWN[0x5] = true;
					break;
				case SDLK_e:
					ctx->KEYDOWN[0x6] = true;
					break;
				case SDLK_r:
					ctx->KEYDOWN[0xd] = true;
					break;
				case SDLK_a:
					ctx->KEYDOWN[0x7] = true;
					break;
				case SDLK_s:
					ctx->KEYDOWN[0x8] = true;
					break;
				case SDLK_d:
					ctx->KEYDOWN[0x9] = true;
					break;
				case SDLK_f:
					ctx->KEYDOWN[0xe] = true;
					break;
				case SDLK_z:
					ctx->KEYDOWN[0xa] = true;
					break;
				case SDLK_x:
					ctx->KEYDOWN[0x0] = true;
					break;
				case SDLK_c:
					ctx->KEYDOWN[0xb] = true;
					break;
				case SDLK_v:
					ctx->KEYDOWN[0xf] = true;
					break;
				}

				break;

			case SDL_KEYUP:
				switch (e.key.keysym.sym) {
				case SDLK_1:
					ctx->KEYDOWN[0x1] = false;
					break;
				case SDLK_2:
					ctx->KEYDOWN[0x2] = false;
					break;
				case SDLK_3:
					ctx->KEYDOWN[0x3] = false;
					break;
				case SDLK_4:
					ctx->KEYDOWN[0xc] = false;
					break;
				case SDLK_q:
					ctx->KEYDOWN[0x4] = false;
					break;
				case SDLK_w:
					ctx->KEYDOWN[0x5] = false;
					break;
				case SDLK_e:
					ctx->KEYDOWN[0x6] = false;
					break;
				case SDLK_r:
					ctx->KEYDOWN[0xd] = false;
					break;
				case SDLK_a:
					ctx->KEYDOWN[0x7] = false;
					break;
				case SDLK_s:
					ctx->KEYDOWN[0x8] = false;
					break;
				case SDLK_d:
					ctx->KEYDOWN[0x9] = false;
					break;
				case SDLK_f:
					ctx->KEYDOWN[0xe] = false;
					break;
				case SDLK_z:
					ctx->KEYDOWN[0xa] = false;
					break;
				case SDLK_x:
					ctx->KEYDOWN[0x0] = false;
					break;
				case SDLK_c:
					ctx->KEYDOWN[0xb] = false;
					break;
				case SDLK_v:
					ctx->KEYDOWN[0xf] = false;
					break;
				}

				break;

			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		op1 = ctx->RAM[ctx->PC];
		op2 = ctx->RAM[ctx->PC + 1];
		opcode = ((uint16_t)op1 << 8) | (uint16_t)op2;
		nnn = ((uint16_t)op1 & 0x0f) << 8;
		nnn |= (uint16_t)op2;

		if ((op1 & 0xf0) == 0x00) {
			if (opcode == 0x00e0) {
#if DEBUG
				printf("%04X: CLS\n", ctx->PC);
#endif
				memset(ctx->SCREEN, 0, sizeof(ctx->SCREEN));
				ctx->PC += 2;
			} else if (opcode == 0x00ee) {
#if DEBUG
				printf("%04X: RET\n", ctx->PC);
#endif
				ctx->PC = ctx->STACK[(size_t)ctx->SP];
				ctx->SP -= 1;
			} else {
#if DEBUG
				printf("%04X: SYS #%04X\n", ctx->PC, nnn);
#endif
				ctx->PC = nnn;
			}
		} else if ((op1 & 0xf0) == 0x10) {
#if DEBUG
			printf("%04X: JP #%04X\n", ctx->PC, nnn);
#endif
			ctx->PC = nnn;
		} else if ((op1 & 0xf0) == 0x20) {
#if DEBUG
			printf("%04X: CALL #%04X\n", ctx->PC, nnn);
#endif
			ctx->SP += 1;
			ctx->STACK[(size_t)ctx->SP] = ctx->PC;
			ctx->PC = nnn;
		} else if ((op1 & 0xf0) == 0x30) {
			size_t vx = (size_t)op1 & 0x0f;

#if DEBUG
			printf("%04X: SE V%zu, #%02X\n", ctx->PC, vx, op2);
#endif

			if (ctx->V[vx] == op2)
				ctx->PC += 2;

			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0x40) {
			size_t vx = (size_t)op1 & 0x0f;

#if DEBUG
			printf("%04X: SE V%zu, #%02X\n", ctx->PC, vx, op2);
#endif

			if (ctx->V[vx] != op2)
				ctx->PC += 2;

			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0x50) {
			size_t vx = (size_t)op1 & 0x0f;
			size_t vy = ((size_t)op2 & 0xf0) >> 4;

#if DEBUG
			printf("%04X: SE V%zu, V%zu\n", ctx->PC, vx, vy);
#endif

			if (ctx->V[vx] == ctx->V[vy])
				ctx->PC += 2;

			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0x60) {
			size_t vx = (size_t)op1 & 0x0f;
#if DEBUG
			printf("%04X: LD V%zu, #%02X\n", ctx->PC, vx, op2);
#endif
			ctx->V[vx] = op2;
			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0x70) {
			size_t vx = (size_t)op1 & 0x0f;
#if DEBUG
			printf("%04X: ADD V%zu, #%02X\n", ctx->PC, vx, op2);
#endif
			ctx->V[vx] += op2;
			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0x80) {
			size_t vx = (size_t)op1 & 0x0f;
			size_t vy = ((size_t)op2 & 0xf0) >> 4;

			switch (op2 & 0x0f) {
			case 0x00:
#if DEBUG
				printf("%04X: LD V%zu, V%zu\n", ctx->PC, vx, vy);
#endif
				ctx->V[vx] = ctx->V[vy];
				break;
			case 0x01:
#if DEBUG
				printf("%04X: OR V%zu, V%zu\n", ctx->PC, vx, vy);
#endif
				ctx->V[vx] |= ctx->V[vy];
				break;
			case 0x02:
#if DEBUG
				printf("%04X: AND V%zu, V%zu\n", ctx->PC, vx, vy);
#endif
				ctx->V[vx] &= ctx->V[vy];
				break;
			case 0x03:
#if DEBUG
				printf("%04X: XOR V%zu, V%zu\n", ctx->PC, vx, vy);
#endif
				ctx->V[vx] ^= ctx->V[vy];
				break;
			case 0x04:
			{
#if DEBUG
				printf("%04X: ADD V%zu, V%zu\n", ctx->PC, vx, vy);
#endif
				uint16_t result = (uint16_t)ctx->V[vx] + (uint16_t)ctx->V[vy];
				ctx->V[0xF] = result > 255 ? 1 : 0;
				ctx->V[vx] = (uint8_t)(result & 0x00ff);
				break;
			}
			case 0x05:
#if DEBUG
				printf("%04X: SUB V%zu, V%zu\n", ctx->PC, vx, vy);
#endif
				ctx->V[0xF] = ctx->V[vy] > ctx->V[vx] ? 0 : 1;
				ctx->V[vx] -= ctx->V[vy];
				break;
			case 0x06:
#if DEBUG
				printf("%04X: SHR V%zu\n", ctx->PC, vx);
#endif
				ctx->V[0xF] = ctx->V[vx] & 0x01;
				ctx->V[vx] >>= 1;
				break;
			case 0x07:
#if DEBUG
				printf("%04X: SUBN V%zu, V%zu\n", ctx->PC, vx, vy);
#endif
				ctx->V[0xF] = ctx->V[vy] > ctx->V[vx] ? 1 : 0;
				ctx->V[vx] = ctx->V[vy] - ctx->V[vx];
				break;
			case 0x0e:
#if DEBUG
				printf("%04X: SHL V%zu\n", ctx->PC, vx);
#endif
				ctx->V[0xF] = (ctx->V[vx] & 0x80) >> 7;
				ctx->V[vx] <<= 1;
				break;
			}

			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0x90) {
			size_t vx = op1 & 0x0f;
			size_t vy = (op2 & 0xf0) >> 4;

#if DEBUG
			printf("%04X: SNE V%zu, V%zu\n", ctx->PC, vx, vy);
#endif

			if (ctx->V[vx] != ctx->V[vy])
				ctx->PC += 2;

			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0xa0) {
#if DEBUG
			printf("%04X: LD I, #%04X\n", ctx->PC, nnn);
#endif
			ctx->I = nnn;
			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0xb0) {
#if DEBUG
			printf("%04X: JP V0, #%04X\n", ctx->PC, nnn);
#endif
			ctx->PC = nnn + (uint16_t)ctx->V[0];
		} else if ((op1 & 0xf0) == 0xc0) {
			size_t vx = op1 & 0x0f;
#if DEBUG
			printf("%04X: RND V%zu, #%04X\n", ctx->PC, vx, op2);
#endif
			uint8_t rnd = (uint8_t) rand();
			ctx->V[vx] = rnd & op2;
			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0xd0) {
			uint8_t vx = ctx->V[(size_t)op1 & 0x0f];
			uint8_t vy = ctx->V[((size_t)op2 & 0xf0) >> 4];
			uint8_t n_sprites = op2 & 0x0f;

#if DEBUG
			size_t x = (size_t)op1 & 0x0f;
			size_t y = ((size_t)op2 & 0xf0) >> 4;
			printf("%04X: DRW V%zu (%hhu), V%zu (%hhu), #%02X\n", ctx->PC, x, vx, y, vy, n_sprites);
#endif

			ctx->V[0xF] = 0;
			for (size_t y = 0; y < (size_t)n_sprites; y += 1) {
				uint8_t sprite = ctx->RAM[ctx->I + y];

				for (size_t x = 0; x < 8; x += 1) {
					bool val = ((sprite >> (8 - x)) & 1) == 1;
					size_t screen_x = vx + x;
					size_t screen_y = vy + y;

					ctx->SCREEN[screen_y][screen_x] ^= val;

					if (ctx->SCREEN[screen_y][screen_x] == false) {
						ctx->V[0xF] = 1;
					}
				}
			}

			ctx->PC += 2;
		} else if ((op1 & 0xf0) == 0xe0) {
			size_t vx = (size_t)op1 & 0x0f;
			size_t idx = (size_t)ctx->V[vx];

			switch (op2) {
			case 0x9e:
#if DEBUG
				printf("%04X: SKP V%zu\n", ctx->PC, vx);
#endif
				if (ctx->KEYDOWN[idx])
					ctx->PC += 2;
				break;
			case 0xa1:
#if DEBUG
				printf("%04X: SKNP V%zu\n", ctx->PC, vx);
#endif
				if (!ctx->KEYDOWN[idx])
					ctx->PC += 2;
				break;
			}
		} else if ((op1 & 0xf0) == 0xf0) {
			size_t vx = (size_t)op1 & 0x0f;

			switch (op2) {
			case 0x07:
			{
#if DEBUG
				printf("%04X: LD V%zu, DT\n", ctx->PC, vx);
#endif
				ctx->V[vx] = ctx->DT;
				break;
			}
			case 0x0a:
			{
				// LD Vx, K
				printf("Wait for a key press, store the value of the key in Vx.\n");
				exit(1);
				break;
			}
			case 0x15:
			{
#if DEBUG
				printf("%04X: LD DT, V%zu\n", ctx->PC, vx);
#endif
				ctx->DT = ctx->V[vx];
				break;
			}
			case 0x18:
			{
#if DEBUG
				printf("%04X: LD ST, V%zu\n", ctx->PC, vx);
#endif
				ctx->ST = ctx->V[vx];
				break;
			}
			case 0x1e:
			{
#if DEBUG
				printf("%04X: ADD I, V%zu\n", ctx->PC, vx);
#endif
				ctx->I += ctx->V[vx];
				break;
			}
			case 0x29:
			{
				ctx->I = ctx->V[vx] * 5;
#if DEBUG
				printf("%04X: LD F, V%zu\n", ctx->PC, vx);
#endif
				break;
			}
			case 0x33:
			{
#if DEBUG
				printf("%04X: LD B, V%zu\n", ctx->PC, vx);
#endif

				uint8_t hundreds = ctx->V[vx] / 100;
				ctx->RAM[ctx->I] = hundreds;

				uint8_t tens = (ctx->V[vx] - (hundreds * 100)) / 10;
				ctx->RAM[ctx->I + 1] = tens;

				uint8_t units = ctx->V[vx] - (hundreds * 100) - (tens * 10);
				ctx->RAM[ctx->I + 2] = units;

				break;
			}
			case 0x55:
			{
#if DEBUG
				printf("%04X: LD [I], V%zu\n", ctx->PC, vx);
#endif
				for (size_t i = 0; i <= (size_t)vx; i += 1)
					ctx->RAM[ctx->I + i] = ctx->V[i];
				break;
			}
			case 0x65:
			{
#if DEBUG
				printf("%04X: LD V%zu, [I]\n", ctx->PC, vx);
#endif
				for (size_t i = 0; i <= (size_t)vx; i += 1)
					ctx->V[i] = ctx->RAM[ctx->I + i];
				break;
			}
			}

			ctx->PC += 2;
		}

		for (size_t y = 0; y < SCREEN_HEIGHT; y += 1) {
			for (size_t x = 0; x < SCREEN_WIDTH; x += 1) {
				int rgb = ctx->SCREEN[y][x] ? 255 : 0;

				SDL_Rect square = {
					.x = x * 10,
					.y = y * 10,
					.w = 10,
					.h = 10,
				};

				SDL_FillRect(
					surface,
					&square,
					SDL_MapRGB(surface->format, rgb, rgb, rgb)
				);
			}
		}

		SDL_UpdateWindowSurface(window);
		SDL_Delay(25);
	}
}

void machine_init(struct Machine *ctx, uint8_t *prg_rom, size_t prg_size)
{
	srand(0);

	// Zero the entire damn thing first
	memset(ctx, 0, sizeof(*ctx));

	// Copy the program ROM into RAM at 0x200 and set the program counter
	memcpy(&ctx->RAM[0x200], prg_rom, prg_size);
	ctx->PC = 0x200;

	// TODO stack starts at 0xea0 apparently...
	ctx->SP = 0x000;

	/* 
	 * Programs may also refer to a group of sprites representing the
	 * hexadecimal digits 0 through F. These sprites are 5 bytes long, or
	 * 8x5 pixels. The data should be stored in the interpreter area of
	 * Chip-8 memory (0x000 to 0x1FF).
	 */

	// 0
	ctx->RAM[0x000] = 0xf0;
	ctx->RAM[0x001] = 0x90;
	ctx->RAM[0x002] = 0x90;
	ctx->RAM[0x003] = 0x90;
	ctx->RAM[0x004] = 0xf0;

	// 1
	ctx->RAM[0x005] = 0x20;
	ctx->RAM[0x006] = 0x60;
	ctx->RAM[0x007] = 0x20;
	ctx->RAM[0x008] = 0x20;
	ctx->RAM[0x009] = 0x70;

	// 2
	ctx->RAM[0x00a] = 0xf0;
	ctx->RAM[0x00b] = 0x10;
	ctx->RAM[0x00c] = 0xf0;
	ctx->RAM[0x00d] = 0x80;
	ctx->RAM[0x00e] = 0xf0;

	// 3
	ctx->RAM[0x00f] = 0xf0;
	ctx->RAM[0x010] = 0x10;
	ctx->RAM[0x011] = 0xf0;
	ctx->RAM[0x012] = 0x10;
	ctx->RAM[0x013] = 0xf0;

	// 4
	ctx->RAM[0x014] = 0x90;
	ctx->RAM[0x015] = 0x90;
	ctx->RAM[0x016] = 0xf0;
	ctx->RAM[0x017] = 0x10;
	ctx->RAM[0x018] = 0x10;

	// 5
	ctx->RAM[0x019] = 0xf0;
	ctx->RAM[0x01a] = 0x80;
	ctx->RAM[0x01b] = 0xf0;
	ctx->RAM[0x01c] = 0x10;
	ctx->RAM[0x01d] = 0xf0;

	// 6
	ctx->RAM[0x01e] = 0xf0;
	ctx->RAM[0x01f] = 0x80;
	ctx->RAM[0x020] = 0xf0;
	ctx->RAM[0x021] = 0x90;
	ctx->RAM[0x022] = 0xf0;

	// 7
	ctx->RAM[0x023] = 0xf0;
	ctx->RAM[0x024] = 0x10;
	ctx->RAM[0x025] = 0x20;
	ctx->RAM[0x026] = 0x40;
	ctx->RAM[0x027] = 0x40;

	// 8
	ctx->RAM[0x028] = 0xf0;
	ctx->RAM[0x029] = 0x90;
	ctx->RAM[0x02a] = 0xf0;
	ctx->RAM[0x02b] = 0x90;
	ctx->RAM[0x02c] = 0xf0;

	// 9
	ctx->RAM[0x02d] = 0xf0;
	ctx->RAM[0x02e] = 0x90;
	ctx->RAM[0x02f] = 0xf0;
	ctx->RAM[0x030] = 0x10;
	ctx->RAM[0x031] = 0xf0;

	// A
	ctx->RAM[0x032] = 0xf0;
	ctx->RAM[0x033] = 0x90;
	ctx->RAM[0x034] = 0xf0;
	ctx->RAM[0x035] = 0x90;
	ctx->RAM[0x036] = 0x90;

	// B
	ctx->RAM[0x037] = 0xe0;
	ctx->RAM[0x038] = 0x90;
	ctx->RAM[0x039] = 0xe0;
	ctx->RAM[0x03a] = 0x90;
	ctx->RAM[0x03b] = 0xe0;

	// C
	ctx->RAM[0x03c] = 0xf0;
	ctx->RAM[0x03d] = 0x80;
	ctx->RAM[0x03e] = 0x80;
	ctx->RAM[0x03f] = 0x80;
	ctx->RAM[0x040] = 0xf0;

	// D
	ctx->RAM[0x041] = 0xe0;
	ctx->RAM[0x042] = 0x90;
	ctx->RAM[0x043] = 0x90;
	ctx->RAM[0x044] = 0x90;
	ctx->RAM[0x045] = 0xe0;

	// E
	ctx->RAM[0x046] = 0xf0;
	ctx->RAM[0x047] = 0x80;
	ctx->RAM[0x048] = 0xf0;
	ctx->RAM[0x049] = 0x80;
	ctx->RAM[0x04a] = 0xf0;

	// F
	ctx->RAM[0x04b] = 0xf0;
	ctx->RAM[0x04c] = 0x80;
	ctx->RAM[0x04d] = 0xf0;
	ctx->RAM[0x04e] = 0x80;
	ctx->RAM[0x04f] = 0x80;
}
