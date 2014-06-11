#include <stdio.h>
#include <stdint.h>
#include "memory.h"

static uint16_t SP, PC;
static uint8_t regs[8];
static uint16_t *AF = (uint16_t*)&regs[0], *BC = (uint16_t*)&regs[2],
	*DE = (uint16_t*)&regs[4], *HL = (uint16_t*)&regs[6];

static uint8_t *F = (uint8_t*)&regs[0], *A = (uint8_t*)&regs[1],
	*C = (uint8_t*)&regs[2], *B = (uint8_t*)&regs[3],
	*E = (uint8_t*)&regs[4], *D = (uint8_t*)&regs[5],
	*L = (uint8_t*)&regs[6], *H = (uint8_t*)&regs[7];

void cpu_init() {
	PC = 0;
}

void cpu_op_ld_8(uint8_t *dst, uint8_t src) {
	*dst = src;
}

void cpu_op_ld_16(uint16_t *dst, uint16_t src) {
	*dst = src;
}

int cpu_step() {
	uint8_t op;
	uint8_t d8, a8, r8;
	uint16_t d16, a16;

	op = mem_read_8(PC);
	d8 = mem_read_8(PC + 1);
	a8 = d8 + 0xFF00;
	d16 = mem_read_16(PC + 1);
	a16 = d16;
	r8 = (uint8_t)((int8_t)mem_read_8(PC + 1) + PC + 2);

	switch (op) {
	case 0x76:
		// HALT
		return 4;
		break;
	}
	switch (op && 0xF0 >> 4) {
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		return -1;
		// LD
		
	}
}

void cpu_dump_reg() {
	printf("PC: %04x\n", PC);
	printf("SP: %04x\n", SP);
	printf("A  F\n");
	printf("%02x %02x\n", *A, *F);
	printf("B  C\n");
	printf("%02x %02x\n", *B, *C);
	printf("D  E\n");
	printf("%02x %02x\n", *D, *E);
	printf("H  L\n");
	printf("%02x %02x\n", *H, *L);
}

void cpu_test() {
	cpu_dump_reg();
	PC = 0x000A;
	cpu_step();

	cpu_op_ld_8(A, 0x10);
	cpu_op_ld_16(HL, 0xFFAA);
	cpu_op_ld_8(B, *H);
	cpu_op_ld_16(DE, *HL);
	
	printf("\n");
	cpu_dump_reg();
}
