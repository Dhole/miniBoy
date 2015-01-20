#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "keypad.h"
#include "io_regs.h"
#include "memory.h"

static uint8_t keypad[8];
static uint8_t p1_reg;

uint8_t keypad_read_8(uint16_t addr) {
	p1_reg &= 0xF0;

	p1_reg |= (((p1_reg & 0x10) || !keypad[KEYPAD_RIGHT]) & 
		((p1_reg & 0x20) || !keypad[KEYPAD_A])) ? (0x01 << 0) : 0;
	p1_reg |= (((p1_reg & 0x10) || !keypad[KEYPAD_LEFT]) & 
		((p1_reg & 0x20) || !keypad[KEYPAD_B])) ? (0x01 << 1) : 0;
	p1_reg |= (((p1_reg & 0x10) || !keypad[KEYPAD_UP]) & 
		((p1_reg & 0x20) || !keypad[KEYPAD_SELECT])) ? (0x01 << 2) : 0;
	p1_reg |= (((p1_reg & 0x10) || !keypad[KEYPAD_DOWN]) & 
		((p1_reg & 0x20) || !keypad[KEYPAD_START])) ? (0x01 << 3) : 0;

	switch (addr) {
	case IO_JOYPAD:
		return p1_reg | 0xC0;
		break;
	default:
		break;
	}
	return 0;
}

void keypad_write_8(uint16_t addr, uint8_t v) {
	switch (addr) {
	case IO_JOYPAD:
		p1_reg = (p1_reg & 0xCF) | (v & 0x30);
		break;
	default:
		break;
	}
}

void keypad_emulate(uint8_t *keypad_state) {
	if (!(p1_reg & 0x10)) {
		if ((!keypad_state[KEYPAD_RIGHT] && keypad[KEYPAD_RIGHT]) ||
		    (!keypad_state[KEYPAD_LEFT] && keypad[KEYPAD_LEFT]) ||
		    (!keypad_state[KEYPAD_UP] && keypad[KEYPAD_UP]) ||
		    (!keypad_state[KEYPAD_DOWN] && keypad[KEYPAD_DOWN])) {
			mem_bit_set(IO_IFLAGS, MASK_IO_INT_High_to_Low_P10_P13);
		}
	}
	if (!(p1_reg & 0x20)) {
		if ((!keypad_state[KEYPAD_A] && keypad[KEYPAD_A]) ||
		    (!keypad_state[KEYPAD_B] && keypad[KEYPAD_B]) ||
		    (!keypad_state[KEYPAD_SELECT] && keypad[KEYPAD_SELECT]) ||
		    (!keypad_state[KEYPAD_START] && keypad[KEYPAD_START])) {
			mem_bit_set(IO_IFLAGS, MASK_IO_INT_High_to_Low_P10_P13);
		}

	}

	memcpy(keypad, keypad_state, sizeof(keypad));
}
