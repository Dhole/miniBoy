#include <stdio.h>
#include <stdint.h>
#include "memory.h"

#define OP(n) ops[n].handler(ops[n].a, ops[n].b);

static uint16_t SP, PC;
static uint8_t regs[8];
static uint16_t *AF = (uint16_t*)&regs[0], *BC = (uint16_t*)&regs[2],
	*DE = (uint16_t*)&regs[4], *HL = (uint16_t*)&regs[6];

static uint8_t *F = (uint8_t*)&regs[0], *A = (uint8_t*)&regs[1],
	*C = (uint8_t*)&regs[2], *B = (uint8_t*)&regs[3],
	*E = (uint8_t*)&regs[4], *D = (uint8_t*)&regs[5],
	*L = (uint8_t*)&regs[6], *H = (uint8_t*)&regs[7];

//uint8_t cpu_ops_cycles[256];
//uint8_t cpu_ops_cb_cycles[256];

typedef struct {
	void *a;
	void *b;
	void (*handler)(void*, void*);
	uint8_t cycles;
} opcode;

static opcode ops[4];

/*
void cpu_op_ld_8(uint8_t *dst, uint8_t src) {
	*dst = src;
}

void cpu_op_ld_16(uint16_t *dst, uint16_t src) {
	*dst = src;
}
*/

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
	*C = 0x11;
	cpu_dump_reg();
	PC = 0x000A;
	cpu_step();

	//ops[0x41].handler(ops[0x41].a, ops[0x41].b);
	OP(0x41);
	/*
	cpu_op_ld_8(A, 0x10);
	cpu_op_ld_16(HL, 0xFFAA);
	cpu_op_ld_8(B, *H);
	cpu_op_ld_16(DE, *HL);
	*/
	
	
	printf("\n");
	cpu_dump_reg();
}

void op_ld_8r_8r(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a = *b;
}

void op_ld_16a_8r(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	mem_write_8(*a, *b);
}

void op_ld_8r_16a(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	*a = mem_read_8(*b);
}

void op_ld_16r_16a(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	*a = mem_read_16(*b);
}

void op_halt(void *_a, void *_b) {

}

#define SET_OP(i, f, _a, _b, c)			\
	ops[i].handler = &f;			\
	ops[i].a = _a;				\
	ops[i].b = _b;				\
	ops[i].cycles = c;

void set_opcodes() {
	SET_OP(0x40, op_ld_8r_8r, B, B, 4);
	SET_OP(0x41, op_ld_8r_8r, B, C, 4);
	SET_OP(0x42, op_ld_8r_8r, B, D, 4);
	SET_OP(0x43, op_ld_8r_8r, B, E, 4);
	SET_OP(0x44, op_ld_8r_8r, B, H, 4);
	SET_OP(0x45, op_ld_8r_8r, B, L, 4);
	SET_OP(0x46, op_ld_8r_16a, B, HL, 8);
	SET_OP(0x47, op_ld_8r_8r, B, A, 4);
	SET_OP(0x48, op_ld_8r_8r, C, B, 4);
	SET_OP(0x49, op_ld_8r_8r, C, C, 4);
	SET_OP(0x4A, op_ld_8r_8r, C, D, 4);
	SET_OP(0x4B, op_ld_8r_8r, C, E, 4);
	SET_OP(0x4C, op_ld_8r_8r, C, H, 4);
	SET_OP(0x4D, op_ld_8r_8r, C, L, 4);
	SET_OP(0x4E, op_ld_8r_16a, C, HL, 8);
	SET_OP(0x4F, op_ld_8r_8r, C, A, 4);

	SET_OP(0x50, op_ld_8r_8r, D, B, 4);
	SET_OP(0x51, op_ld_8r_8r, D, C, 4);
	SET_OP(0x52, op_ld_8r_8r, D, D, 4);
	SET_OP(0x53, op_ld_8r_8r, D, E, 4);
	SET_OP(0x54, op_ld_8r_8r, D, H, 4);
	SET_OP(0x55, op_ld_8r_8r, D, L, 4);
	SET_OP(0x56, op_ld_8r_16a, D, HL, 8);
	SET_OP(0x57, op_ld_8r_8r, D, A, 4);
	SET_OP(0x58, op_ld_8r_8r, E, B, 4);
	SET_OP(0x59, op_ld_8r_8r, E, C, 4);
	SET_OP(0x5A, op_ld_8r_8r, E, D, 4);
	SET_OP(0x5B, op_ld_8r_8r, E, E, 4);
	SET_OP(0x5C, op_ld_8r_8r, E, H, 4);
	SET_OP(0x5D, op_ld_8r_8r, E, L, 4);
	SET_OP(0x5E, op_ld_8r_16a, E, HL, 8);
	SET_OP(0x5F, op_ld_8r_8r, E, A, 4);

	SET_OP(0x60, op_ld_8r_8r, H, B, 4);
	SET_OP(0x61, op_ld_8r_8r, H, C, 4);
	SET_OP(0x62, op_ld_8r_8r, H, D, 4);
	SET_OP(0x63, op_ld_8r_8r, H, E, 4);
	SET_OP(0x64, op_ld_8r_8r, H, H, 4);
	SET_OP(0x65, op_ld_8r_8r, H, L, 4);
	SET_OP(0x66, op_ld_8r_16a, H, HL, 8);
	SET_OP(0x67, op_ld_8r_8r, H, A, 4);
	SET_OP(0x68, op_ld_8r_8r, L, B, 4);
	SET_OP(0x69, op_ld_8r_8r, L, C, 4);
	SET_OP(0x6A, op_ld_8r_8r, L, D, 4);
	SET_OP(0x6B, op_ld_8r_8r, L, E, 4);
	SET_OP(0x6C, op_ld_8r_8r, L, H, 4);
	SET_OP(0x6D, op_ld_8r_8r, L, L, 4);
	SET_OP(0x6E, op_ld_8r_16a, L, HL, 8);
	SET_OP(0x6F, op_ld_8r_8r, L, A, 4);

	SET_OP(0x70, op_ld_16a_8r, HL, B, 8);
	SET_OP(0x71, op_ld_16a_8r, HL, C, 8);
	SET_OP(0x72, op_ld_16a_8r, HL, D, 8);
	SET_OP(0x73, op_ld_16a_8r, HL, E, 8);
	SET_OP(0x74, op_ld_16a_8r, HL, H, 8);
	SET_OP(0x75, op_ld_16a_8r, HL, L, 8);
	SET_OP(0x76, op_halt, NULL, NULL, 4);
	SET_OP(0x77, op_ld_16a_8r, HL, A, 8);
	SET_OP(0x78, op_ld_8r_8r, A, B, 4);
	SET_OP(0x79, op_ld_8r_8r, A, C, 4);
	SET_OP(0x7A, op_ld_8r_8r, A, D, 4);
	SET_OP(0x7B, op_ld_8r_8r, A, E, 4);
	SET_OP(0x7C, op_ld_8r_8r, A, H, 4);
	SET_OP(0x7D, op_ld_8r_8r, A, L, 4);
	SET_OP(0x7E, op_ld_8r_16a, A, HL, 8);
	SET_OP(0x7F, op_ld_8r_8r, A, A, 4);
}

void cpu_init() {

	set_opcodes();
	PC = 0;
}
