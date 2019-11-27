#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"

bool machine_tick(struct Machine *ctx)
{
	if (ctx->waiting_for_keypress)
		return false;

	bool should_render = false;
	uint8_t op1 = ctx->RAM[ctx->PC];
	uint8_t op2 = ctx->RAM[ctx->PC + 1];
	uint16_t opcode = ((uint16_t)op1 << 8) | (uint16_t)op2;
	uint16_t nnn = ((uint16_t)op1 & 0x0f) << 8;
	nnn |= (uint16_t)op2;

	switch (op1 & 0xf0) {
	case 0x00:
	{
		if (opcode == 0x00e0) {
#if DEBUG
			printf("%04X: CLS\n", ctx->PC);
#endif
			memset(ctx->SCREEN, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
			ctx->PC += 2;
		} else if (opcode == 0x00ee) {
#if DEBUG
			printf("%04X: RET\n", ctx->PC);
#endif
			ctx->SP -= 1;
			ctx->PC = ctx->STACK[(size_t)ctx->SP];
		} else {
#if DEBUG
			printf("%04X: SYS #%04X\n", ctx->PC, nnn);
#endif
			ctx->PC = nnn;
		}

		break;
	}

	case 0x10:
	{
#if DEBUG
		printf("%04X: JP #%04X\n", ctx->PC, nnn);
#endif
		ctx->PC = nnn;
		break;
	}
	
	case 0x20:
	{
#if DEBUG
		printf("%04X: CALL #%04X\n", ctx->PC, nnn);
#endif
		ctx->STACK[(size_t)ctx->SP] = ctx->PC + 2;
		ctx->SP += 1;
		ctx->PC = nnn;
		break;
	}

	case 0x30:
	{
		size_t vx = (size_t)op1 & 0x0f;

#if DEBUG
		printf("%04X: SE V%zu, #%02X\n", ctx->PC, vx, op2);
#endif

		if (ctx->V[vx] == op2)
			ctx->PC += 2;

		ctx->PC += 2;
		break;
	}

	case 0x40:
	{
		size_t vx = (size_t)op1 & 0x0f;

#if DEBUG
		printf("%04X: SE V%zu, #%02X\n", ctx->PC, vx, op2);
#endif

		if (ctx->V[vx] != op2)
			ctx->PC += 2;

		ctx->PC += 2;
		break;
	}

	case 0x50:
	{
		size_t vx = (size_t)op1 & 0x0f;
		size_t vy = ((size_t)op2 & 0xf0) >> 4;

#if DEBUG
		printf("%04X: SE V%zu, V%zu\n", ctx->PC, vx, vy);
#endif

		if (ctx->V[vx] == ctx->V[vy])
			ctx->PC += 2;

		ctx->PC += 2;
		break;
	}

	case 0x60:
	{
		size_t vx = (size_t)op1 & 0x0f;
#if DEBUG
		printf("%04X: LD V%zu, #%02X\n", ctx->PC, vx, op2);
#endif
		ctx->V[vx] = op2;
		ctx->PC += 2;
		break;
	}

	case 0x70:
	{
		size_t vx = (size_t)op1 & 0x0f;
#if DEBUG
		printf("%04X: ADD V%zu, #%02X\n", ctx->PC, vx, op2);
#endif
		ctx->V[vx] += op2;
		ctx->PC += 2;
		break;
	}

	case 0x80:
	{
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
		break;
	}

	case 0x90:
	{
		size_t vx = op1 & 0x0f;
		size_t vy = (op2 & 0xf0) >> 4;

#if DEBUG
		printf("%04X: SNE V%zu, V%zu\n", ctx->PC, vx, vy);
#endif

		if (ctx->V[vx] != ctx->V[vy])
			ctx->PC += 2;

		ctx->PC += 2;
		break;
	}

	case 0xa0:
	{
#if DEBUG
		printf("%04X: LD I, #%04X\n", ctx->PC, nnn);
#endif
		ctx->I = nnn;
		ctx->PC += 2;
		break;
	}

	case 0xb0:
	{
#if DEBUG
		printf("%04X: JP V0, #%04X\n", ctx->PC, nnn);
#endif
		ctx->PC = nnn + (uint16_t)ctx->V[0];
		break;
	}

	case 0xc0:
	{
		size_t vx = op1 & 0x0f;
#if DEBUG
		printf("%04X: RND V%zu, #%04X\n", ctx->PC, vx, op2);
#endif
		uint8_t rnd = (uint8_t) rand();
		ctx->V[vx] = rnd & op2;
		ctx->PC += 2;
		break;
	}

	case 0xd0:
	{
		uint8_t vx = ctx->V[(size_t)op1 & 0x0f];
		uint8_t vy = ctx->V[((size_t)op2 & 0xf0) >> 4];
		uint8_t n_rows = op2 & 0x0f;

#if DEBUG
		size_t x = (size_t)op1 & 0x0f;
		size_t y = ((size_t)op2 & 0xf0) >> 4;
		printf("%04X: DRW V%zu (%hhu), V%zu (%hhu), #%02X\n",
			ctx->PC, x, vx, y, vy, n_rows);
#endif

		ctx->V[0xF] = 0;
		for (size_t y = 0; y < (size_t)n_rows; y += 1) {
			uint8_t sprite_row = ctx->RAM[ctx->I + y];
			size_t screen_y = vy + y;

			for (size_t x = 0; x < 8; x += 1) {
				bool val = ((sprite_row >> (7 - x)) & 1) == 1;
				size_t screen_x = vx + x;

				// Collision detection is determined by
				// checking if a pixel that is on is about to
				// be switched off.
				ctx->V[0xF] |= ctx->SCREEN[screen_y][screen_x] && val;

				ctx->SCREEN[screen_y][screen_x] ^= val;
			}
		}

		should_render = true;
		ctx->PC += 2;
		break;
	}

	case 0xe0:
	{
		size_t vx = (size_t)op1 & 0x0f;
		size_t idx = (size_t)ctx->V[vx];

		switch (op2) {
		case 0x9e:
#if DEBUG
			printf("%04X: SKP V%zu\n", ctx->PC, vx);
#endif
			if (ctx->KEYDOWN[idx])
				ctx->PC += 2;

			ctx->PC += 2;
			break;
		case 0xa1:
#if DEBUG
			printf("%04X: SKNP V%zu\n", ctx->PC, vx);
#endif
			if (!ctx->KEYDOWN[idx])
				ctx->PC += 2;

			ctx->PC += 2;
			break;
		}

		break;
	}

	case 0xf0:
	{
		size_t vx = (size_t)op1 & 0x0f;

		switch (op2) {
		case 0x07:
#if DEBUG
			printf("%04X: LD V%zu, DT\n", ctx->PC, vx);
#endif
			ctx->V[vx] = ctx->DT;
			break;
		case 0x0a:
			// LD Vx, K
#if DEBUG
			printf("%04X: LD V%zu, K\n", ctx->PC, vx);
#endif
			ctx->waiting_for_keypress = true;
			ctx->key_wait_reg = vx;
			break;
		case 0x15:
#if DEBUG
			printf("%04X: LD DT, V%zu\n", ctx->PC, vx);
#endif
			ctx->DT = ctx->V[vx];
			break;
		case 0x18:
#if DEBUG
			printf("%04X: LD ST, V%zu\n", ctx->PC, vx);
#endif
			ctx->ST = ctx->V[vx];
			break;
		case 0x1e:
#if DEBUG
			printf("%04X: ADD I, V%zu\n", ctx->PC, vx);
#endif
			ctx->I += ctx->V[vx];
			break;
		case 0x29:
			ctx->I = ctx->V[vx] * 5;
#if DEBUG
			printf("%04X: LD F, V%zu\n", ctx->PC, vx);
#endif
			break;
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
#if DEBUG
			printf("%04X: LD [I], V%zu\n", ctx->PC, vx);
#endif
			for (size_t i = 0; i <= (size_t)vx; i += 1)
				ctx->RAM[ctx->I + i] = ctx->V[i];
			break;
		case 0x65:
#if DEBUG
			printf("%04X: LD V%zu, [I]\n", ctx->PC, vx);
#endif
			for (size_t i = 0; i <= (size_t)vx; i += 1)
				ctx->V[i] = ctx->RAM[ctx->I + i];
			break;
		}

		ctx->PC += 2;
		break;
	}

	} // switch

	return should_render;
}

void machine_keydown(struct Machine *ctx, size_t key)
{
	ctx->KEYDOWN[key] = true;

	if (ctx->waiting_for_keypress) {
		ctx->waiting_for_keypress = false;
		ctx->V[ctx->key_wait_reg] = key;
	}
}

void machine_keyup(struct Machine *ctx, size_t key)
{
	ctx->KEYDOWN[key] = false;
}

void machine_tick_timers(struct Machine *ctx)
{
	if (ctx->DT > 0) {
		ctx->DT -= 1;
	}

	if (ctx->ST > 0) {
		ctx->ST -= 1;
	}
}

void machine_init(struct Machine *ctx, uint8_t *prg_rom, size_t prg_size)
{
	srand(0);

	// Zero the entire damn thing first
	memset(ctx, 0, sizeof(*ctx));

	// Copy the program ROM into RAM and set the program counter
	memcpy(&ctx->RAM[PRGROM_ADDRESS], prg_rom, prg_size);
	ctx->PC = PRGROM_ADDRESS;

	// The stack is a collection of 16-bit values in RAM
	ctx->STACK = (uint16_t *)&ctx->RAM[STACK_ADDRESS];
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
