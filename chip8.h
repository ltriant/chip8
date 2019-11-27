#pragma once

#include <stdbool.h>
#include <stdint.h>

// Prints disassembly to stdout
#define DEBUG 0

// 64x32 display, with the origin at the top-left of the screen
#define SCREEN_WIDTH  64
#define SCREEN_HEIGHT 32

// Clock rate, in Hz
#define CLOCK_RATE 500.0
#define US_PER_CLOCK ((int)((1.0 / CLOCK_RATE) * 1000.0 * 1000.0))

// Clock rate of the sound and delay timers, in Hz
#define TIMER_RATE 60.0
#define US_PER_TIMER_CLOCK ((int)((1.0 / TIMER_RATE) * 1000.0 * 1000.0))

#define PRGROM_ADDRESS 0x200
#define STACK_ADDRESS  0xEA0
#define SCREEN_ADDRESS 0xF00

struct Machine {
	bool SCREEN[SCREEN_HEIGHT][SCREEN_WIDTH];
	bool KEYDOWN[16];

	/*
	 * Memory Map:
	 * +---------------+= 0xFFF (4095) End of Chip-8 RAM
	 * |               |
	 * |               |= 0xF00 (3840) Start of screen
	 * |               |= 0xEA0 (3744) Start of stack
	 * |               |
	 * |               |
	 * | 0x200 to 0xFFF|
	 * |     Chip-8    |
	 * | Program / Data|
	 * |     Space     |
	 * |               |
	 * |               |
	 * |               |
	 * +- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
	 * |               |
	 * |               |
	 * |               |
	 * +---------------+= 0x200 (512) Start of most Chip-8 programs
	 * | 0x000 to 0x1FF|
	 * | Reserved for  |
	 * |  interpreter  |
	 * +---------------+= 0x000 (0) Start of Chip-8 RAM
	 */
	uint8_t RAM[4096];

	/*
	 * The stack is an array of 16-bit values, used to store the address
	 * that the interpreter should return to when finished with a
	 * subroutine.
	 */
	uint16_t *STACK;

	// 16 general purpose 8-bit registers
	uint8_t V[16];

	/*
	 * When we're waiting for an input key, this is the register that the
	 * key gets written to.
	 */
	bool waiting_for_keypress;
	size_t key_wait_reg;

	/*
	 * This register is generally used to store memory addresses, so only
	 * the lowest (rightmost) 12 bits are usually used.
	 */
	uint16_t I;

	// Sound timer. Decrements at 60Hz when non-zero.
	uint8_t ST;

	// Delay timer. Decrements at 60Hz when non-zero.
	uint8_t DT;

	// Program counter
	uint16_t PC;

	// Stack pointer
	uint16_t SP;
};

void machine_init(struct Machine *ctx, uint8_t *prg_rom, size_t prg_size);
bool machine_tick(struct Machine *ctx);
void machine_keydown(struct Machine *ctx, size_t key);
void machine_keyup(struct Machine *ctx, size_t key);
void machine_tick_timers(struct Machine *ctx);
