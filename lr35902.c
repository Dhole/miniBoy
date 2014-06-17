#include <stdio.h>
#include <stdint.h>
#include "memory.h"

#define OP(n) ops[n].handler(ops[n].a, ops[n].b);

static uint16_t SP_val, PC;
static uint16_t *SP = &SP_val;
static uint8_t regs[8];
static uint16_t *AF = (uint16_t*)&regs[0], *BC = (uint16_t*)&regs[2],
	*DE = (uint16_t*)&regs[4], *HL = (uint16_t*)&regs[6];

static uint8_t *F = (uint8_t*)&regs[0], *A = (uint8_t*)&regs[1],
	*C = (uint8_t*)&regs[2], *B = (uint8_t*)&regs[3],
	*E = (uint8_t*)&regs[4], *D = (uint8_t*)&regs[5],
	*L = (uint8_t*)&regs[6], *H = (uint8_t*)&regs[7];

typedef enum {
	Z_FLAG = 1 << 7,
	N_FLAG = 1 << 6,
	H_FLAG = 1 << 5,
	C_FLAG = 1 << 4,
} flag_t;

static uint16_t pc1_val;
static uint16_t *pc1 = &pc1_val;

//uint8_t cpu_ops_cycles[256];
//uint8_t cpu_ops_cb_cycles[256];

typedef struct {
	char *desc;
	void (*handler)(void*, void*);
	void *a;
	void *b;
	uint8_t cycles;
} opcode;

static opcode ops[256];

/*
void cpu_op_ld_8(uint8_t *dst, uint8_t src) {
	*dst = src;
}

void cpu_op_ld_16(uint16_t *dst, uint16_t src) {
	*dst = src;
}
*/

void set_flag(char f, uint8_t v) {
	if (v) {
		*F |= f;
	} else {
		*F &= ~f;
	}
}

void set_flag_Z(uint8_t *a) {
	if (*a == 0) {
		set_flag(Z_FLAG, 1);
	} else {
		set_flag(Z_FLAG, 0);
	}	
}

uint8_t get_flag(flag_t f) {
	return (*F & f) > 0 ? 1 : 0;
}

int cpu_step() {
	uint8_t op;
	//uint8_t d8, a8, r8;
	//uint16_t d16, a16;

	op = mem_read_8(PC);
	printf("0x%04X : %02X\n", PC, op);
	/*
	d8 = mem_read_8(PC + 1);
	a8 = d8 + 0xFF00;
	d16 = mem_read_16(PC + 1);
	a16 = d16;
	r8 = (uint8_t)((int8_t)mem_read_8(PC + 1) + PC + 2);
	*/
	
	*pc1 = PC + 1;

	OP(op);

	return ops[op].cycles;
	
	/*
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
	*/
}

void cpu_dump_reg() {
	printf("PC: %04x\n", PC);
	printf("SP: %04x\n", *SP);
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
	cpu_step();
	
	printf("\n");
	cpu_dump_reg();
}

void op_ld_8r_8r(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a = *b;
}

void op_ld_8r_8a(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a = mem_read_8((uint16_t)*b + 0xFF00);
}

void op_ld_8a_8r(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a = *b;
	mem_write_8((uint16_t)*a + 0xFF00, *b);
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
	//printf(">> %04x \n", mem_read_16(*b));
	*a = mem_read_16(*b);
}

void op_ld_16a_16r(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	mem_write_16(*a, *b);
}

void op_ld_16a_16a(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	mem_write_8(*a, mem_read_8(*b));
}

void op_ld_16r_16r(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	*a = *b;
}

void op_ld_16a_8r_p(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	mem_write_8(*a, *b);
	*a++;
}

void op_ld_16a_8r_m(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	mem_write_8(*a, *b);
	*a--;
}

void op_ld_8r_16a_p(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	*a = mem_read_8(*b);
	*b++;
}

void op_ld_8r_16a_m(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	*a = mem_read_8(*b);
	*b--;
}

void op_ld_16r_sp_8v(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*HL = *SP + *b;
	// TODO update flags
}

void op_halt(void *_a, void *_b) {
	// TODO
}

void op_add(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a += *b;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0F < *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a > 0xFF - *b) ? 1 : 0);
}

void op_adc(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a += *b + get_flag(C_FLAG);

        set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0F < *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a > 0xFF - *b) ? 1 : 0);
}

void op_sub(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a -= *b;

        set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0F > 0x0F - *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a < *b) ? 1 : 0);
}

void op_sbc(void *_a, void *_b) {
	// TODO
}

void op_and(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a &= *b;

        set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 1);
	set_flag(C_FLAG, 0);
}

void op_xor(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a ^= *b;

        set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, 0);
}

void op_or(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a |= *b;

        set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, 0);
}

void op_cp(void *_a, void *_b) {
	// TODO
}

void op_nop(void *_a, void *_b) {
        
}

#define SET_OP(i, d, f, _a, _b, c)		\
	ops[i].desc = d;			\
	ops[i].handler = &f;			\
	ops[i].a = _a;				\
	ops[i].b = _b;				\
	ops[i].cycles = c;

void set_opcodes() {
	SET_OP(0x00, "NOP", op_nop, NULL, NULL, 4);
	SET_OP(0x01, "LD BC,d16", op_ld_16r_16a, BC, pc1, 12);
	SET_OP(0x02, "LD (BC),A", op_ld_16a_8r, BC, A, 8);
	SET_OP(0x03, "INC BC", op_nop, NULL, NULL, 0);
	SET_OP(0x04, "INC B", op_nop, NULL, NULL, 0);
	SET_OP(0x05, "DEC B", op_nop, NULL, NULL, 0);
	SET_OP(0x06, "LD B,d8", op_ld_8r_16a, B, pc1, 8);
	SET_OP(0x07, "RLCA", op_nop, NULL, NULL, 0);
	SET_OP(0x08, "LD (a16), SP", op_ld_16a_16r, pc1, SP, 20);
	SET_OP(0x09, "ADD HL,BC", op_nop, NULL, NULL, 0);
	SET_OP(0x0A, "LD A,(BC)", op_ld_8r_16a, A, BC, 8);
	SET_OP(0x0B, "DEC BC", op_nop, NULL, NULL, 0);
	SET_OP(0x0C, "INC C", op_nop, NULL, NULL, 0);
	SET_OP(0x0D, "DEC C", op_nop, NULL, NULL, 0);
	SET_OP(0x0E, "LD C,d8", op_ld_8r_16a, C, pc1, 8);
	SET_OP(0x0E, "RRCA", op_nop, NULL, NULL, 0);
	
	SET_OP(0x10, "STOP 0", op_nop, NULL, NULL, 0);
	SET_OP(0x11, "LD DE,d16", op_ld_16r_16a, DE, pc1, 12);
	SET_OP(0x12, "LD (DE),A", op_ld_16a_8r, DE, A, 8);
	SET_OP(0x13, "INC DE", op_nop, NULL, NULL, 0);
	SET_OP(0x14, "INC D", op_nop, NULL, NULL, 0);
	SET_OP(0x15, "DEC D", op_nop, NULL, NULL, 0);
	SET_OP(0x16, "LD D,d8", op_ld_8r_16a, D, pc1, 8);
	SET_OP(0x17, "RLA", op_nop, NULL, NULL, 0);
	SET_OP(0x18, "JR r8", op_nop, NULL, NULL, 0);
	SET_OP(0x19, "ADD HL,DE", op_nop, NULL, NULL, 0);
	SET_OP(0x1A, "LD A,(DE)", op_ld_8r_16a, A, DE, 8);
	SET_OP(0x1B, "DEC DE", op_nop, NULL, NULL, 0);
	SET_OP(0x1C, "INC E", op_nop, NULL, NULL, 0);
	SET_OP(0x1D, "DEC E", op_nop, NULL, NULL, 0);
	SET_OP(0x1E, "LD E,d8", op_ld_8r_16a, E, pc1, 8);
	SET_OP(0x1F, "RRA", op_nop, NULL, NULL, 0);
	
	SET_OP(0x20, "JR NZ,r8", op_nop, NULL, NULL, 0);
	SET_OP(0x21, "LD HL,16", op_ld_16r_16a, HL, pc1, 12);
	SET_OP(0x22, "LD (HL+),A", op_ld_16a_8r_p, HL, A, 8);
	SET_OP(0x23, "INC HL", op_nop, NULL, NULL, 0);
	SET_OP(0x24, "INC H", op_nop, NULL, NULL, 0);
	SET_OP(0x25, "DEC H", op_nop, NULL, NULL, 0);
	SET_OP(0x26, "LD H,d8", op_ld_8r_16a, H, pc1, 8);
	SET_OP(0x27, "DAA", op_nop, NULL, NULL, 0);
	SET_OP(0x28, "JR Z,r8", op_nop, NULL, NULL, 0);
	SET_OP(0x29, "ADD HL,HL", op_nop, NULL, NULL, 0);
	SET_OP(0x2A, "LD A,(HL+)", op_ld_8r_16a_p, A, HL, 8);
	SET_OP(0x2B, "DEC HL", op_nop, NULL, NULL, 0);
	SET_OP(0x2C, "INC L", op_nop, NULL, NULL, 0);
	SET_OP(0x2D, "DEC L", op_nop, NULL, NULL, 0);
	SET_OP(0x2E, "LD L,d8", op_ld_8r_16a, L, pc1, 8);
	SET_OP(0x2F, "CPL", op_nop, NULL, NULL, 0);

	SET_OP(0x30, "JR NC,r8", op_nop, NULL, NULL, 0);
	SET_OP(0x31, "LD SP,16", op_ld_16r_16a, SP, pc1, 12);
	SET_OP(0x32, "LD (HL-),A", op_ld_16a_8r_m, HL, A, 8);
	SET_OP(0x33, "INC SP", op_nop, NULL, NULL, 0);
	SET_OP(0x34, "INC (HL)", op_nop, NULL, NULL, 0);
	SET_OP(0x35, "DEC (HL)", op_nop, NULL, NULL, 0);
	SET_OP(0x36, "LD (HL),d8", op_ld_16a_16a, HL, pc1, 12);
	SET_OP(0x37, "SCF", op_nop, NULL, NULL, 0);
	SET_OP(0x38, "JR C,r8", op_nop, NULL, NULL, 0);
	SET_OP(0x39, "ADD HL, SP", op_nop, NULL, NULL, 0);
	SET_OP(0x3A, "LD A,(HL-)", op_ld_8r_16a_p, A, HL, 8);
	SET_OP(0x3B, "DEC SP", op_nop, NULL, NULL, 0);
	SET_OP(0x3C, "INC A", op_nop, NULL, NULL, 0);
	SET_OP(0x3D, "DEC A", op_nop, NULL, NULL, 0);
	SET_OP(0x3E, "LD A,d8", op_ld_8r_16a, A, pc1, 8);
	SET_OP(0x3F, "CCF", op_nop, NULL, NULL, 0);
	
	SET_OP(0x40, "LD B,B", op_ld_8r_8r, B, B, 4);
	SET_OP(0x41, "LD B,C", op_ld_8r_8r, B, C, 4);
	SET_OP(0x42, "LD B,D", op_ld_8r_8r, B, D, 4);
	SET_OP(0x43, "LD B,E", op_ld_8r_8r, B, E, 4);
	SET_OP(0x44, "LD B,H", op_ld_8r_8r, B, H, 4);
	SET_OP(0x45, "LD B,L", op_ld_8r_8r, B, L, 4);
	SET_OP(0x46, "LD B,(HL)", op_ld_8r_16a, B, HL, 8);
	SET_OP(0x47, "LD B,A", op_ld_8r_8r, B, A, 4);
	SET_OP(0x48, "LD C,B", op_ld_8r_8r, C, B, 4);
	SET_OP(0x49, "LD C,C", op_ld_8r_8r, C, C, 4);
	SET_OP(0x4A, "LD C,D", op_ld_8r_8r, C, D, 4);
	SET_OP(0x4B, "LD C,E", op_ld_8r_8r, C, E, 4);
	SET_OP(0x4C, "LD C,H", op_ld_8r_8r, C, H, 4);
	SET_OP(0x4D, "LD C,L", op_ld_8r_8r, C, L, 4);
	SET_OP(0x4E, "LD C,(HL)", op_ld_8r_16a, C, HL, 8);
	SET_OP(0x4F, "LD C,A", op_ld_8r_8r, C, A, 4);

	SET_OP(0x50, "LD D,B", op_ld_8r_8r, D, B, 4);
	SET_OP(0x51, "LD D,C", op_ld_8r_8r, D, C, 4);
	SET_OP(0x52, "LD D,D", op_ld_8r_8r, D, D, 4);
	SET_OP(0x53, "LD D,E", op_ld_8r_8r, D, E, 4);
	SET_OP(0x54, "LD D,H", op_ld_8r_8r, D, H, 4);
	SET_OP(0x55, "LD D,L", op_ld_8r_8r, D, L, 4);
	SET_OP(0x56, "LD D,(HL)", op_ld_8r_16a, D, HL, 8);
	SET_OP(0x57, "LD D,A", op_ld_8r_8r, D, A, 4);
	SET_OP(0x58, "LD E,B", op_ld_8r_8r, E, B, 4);
	SET_OP(0x59, "LD E,C", op_ld_8r_8r, E, C, 4);
	SET_OP(0x5A, "LD E,D", op_ld_8r_8r, E, D, 4);
	SET_OP(0x5B, "LD E,E", op_ld_8r_8r, E, E, 4);
	SET_OP(0x5C, "LD E,H", op_ld_8r_8r, E, H, 4);
	SET_OP(0x5D, "LD E,L", op_ld_8r_8r, E, L, 4);
	SET_OP(0x5E, "LD E,(HL)", op_ld_8r_16a, E, HL, 8);
	SET_OP(0x5F, "LD E,A", op_ld_8r_8r, E, A, 4);

	SET_OP(0x60, "LD H,B", op_ld_8r_8r, H, B, 4);
	SET_OP(0x61, "LD H,C", op_ld_8r_8r, H, C, 4);
	SET_OP(0x62, "LD H,D", op_ld_8r_8r, H, D, 4);
	SET_OP(0x63, "LD H,E", op_ld_8r_8r, H, E, 4);
	SET_OP(0x64, "LD H,H", op_ld_8r_8r, H, H, 4);
	SET_OP(0x65, "LD H,L", op_ld_8r_8r, H, L, 4);
	SET_OP(0x66, "LD H,(HL)", op_ld_8r_16a, H, HL, 8);
	SET_OP(0x67, "LD H,A", op_ld_8r_8r, H, A, 4);
	SET_OP(0x68, "LD L,B", op_ld_8r_8r, L, B, 4);
	SET_OP(0x69, "LD L,C", op_ld_8r_8r, L, C, 4);
	SET_OP(0x6A, "LD L,D", op_ld_8r_8r, L, D, 4);
	SET_OP(0x6B, "LD L,E", op_ld_8r_8r, L, E, 4);
	SET_OP(0x6C, "LD L,H", op_ld_8r_8r, L, H, 4);
	SET_OP(0x6D, "LD L,L", op_ld_8r_8r, L, L, 4);
	SET_OP(0x6E, "LD L,(HL)", op_ld_8r_16a, L, HL, 8);
	SET_OP(0x6F, "LD L,A", op_ld_8r_8r, L, A, 4);

	SET_OP(0x70, "LD (HL),B", op_ld_16a_8r, HL, B, 8);
	SET_OP(0x71, "LD (HL),C", op_ld_16a_8r, HL, C, 8);
	SET_OP(0x72, "LD (HL),D", op_ld_16a_8r, HL, D, 8);
	SET_OP(0x73, "LD (HL),E", op_ld_16a_8r, HL, E, 8);
	SET_OP(0x74, "LD (HL),H", op_ld_16a_8r, HL, H, 8);
	SET_OP(0x75, "LD (HL),L", op_ld_16a_8r, HL, L, 8);
	SET_OP(0x76, "HALT", op_halt, NULL, NULL, 4);
	SET_OP(0x77, "LD (HL),A", op_ld_16a_8r, HL, A, 8);
	SET_OP(0x78, "LD A,B", op_ld_8r_8r, A, B, 4);
	SET_OP(0x79, "LD A,C", op_ld_8r_8r, A, C, 4);
	SET_OP(0x7A, "LD A,D", op_ld_8r_8r, A, D, 4);
	SET_OP(0x7B, "LD A,E", op_ld_8r_8r, A, E, 4);
	SET_OP(0x7C, "LD A,H", op_ld_8r_8r, A, H, 4);
	SET_OP(0x7D, "LD A,L", op_ld_8r_8r, A, L, 4);
	SET_OP(0x7E, "LD A,(HL)", op_ld_8r_16a, A, HL, 8);
	SET_OP(0x7F, "LD A,A", op_ld_8r_8r, A, A, 4);

	SET_OP(0x80, "ADD A,B", op_add, A, B, 4);
	SET_OP(0x81, "ADD A,C", op_add, A, C, 4);
	SET_OP(0x82, "ADD A,D", op_add, A, D, 4);
	SET_OP(0x83, "ADD A,E", op_add, A, E, 4);
	SET_OP(0x84, "ADD A,H", op_add, A, H, 4);
	SET_OP(0x85, "ADD A,L", op_add, A, L, 4);
	SET_OP(0x86, "ADD A,(HL)", op_add, A, (HL), 8);
	SET_OP(0x87, "ADD A,A", op_add, A, A, 4);
	SET_OP(0x88, "ADC A,B", op_adc, A, B, 4);
	SET_OP(0x89, "ADC A,C", op_adc, A, C, 4);
	SET_OP(0x8A, "ADC A,D", op_adc, A, D, 4);
	SET_OP(0x8B, "ADC A,E", op_adc, A, E, 4);
	SET_OP(0x8C, "ADC A,H", op_adc, A, H, 4);
	SET_OP(0x8D, "ADC A,L", op_adc, A, L, 4);
	SET_OP(0x8E, "ADC A,(HL)", op_adc, A, (HL), 8);
	SET_OP(0x8F, "ADC A,A", op_adc, A, A, 4);
	
	SET_OP(0x90, "SUB B", op_sub, B, NULL, 4);
	SET_OP(0x91, "SUB C", op_sub, C, NULL, 4);
	SET_OP(0x92, "SUB D", op_sub, D, NULL, 4);
	SET_OP(0x93, "SUB E", op_sub, E, NULL, 4);
	SET_OP(0x94, "SUB H", op_sub, H, NULL, 4);
	SET_OP(0x95, "SUB L", op_sub, L, NULL, 4);
	SET_OP(0x96, "SUB (HL)", op_sub, (HL), NULL, 8);
	SET_OP(0x97, "SUB A", op_sub, A, NULL, 4);
	SET_OP(0x98, "SBC A,B", op_sbc, A, B, 4);
	SET_OP(0x99, "SBC A,C", op_sbc, A, C, 4);
	SET_OP(0x9A, "SBC A,D", op_sbc, A, D, 4);
	SET_OP(0x9B, "SBC A,E", op_sbc, A, E, 4);
	SET_OP(0x9C, "SBC A,H", op_sbc, A, H, 4);
	SET_OP(0x9D, "SBC A,L", op_sbc, A, L, 4);
	SET_OP(0x9E, "SBC A,(HL)", op_sbc, A, (HL), 8);
	SET_OP(0x9F, "SBC A,A", op_sbc, A, A, 4);
	
	SET_OP(0xA0, "AND B", op_and, B, NULL, 4);
	SET_OP(0xA1, "AND C", op_and, C, NULL, 4);
	SET_OP(0xA2, "AND D", op_and, D, NULL, 4);
	SET_OP(0xA3, "AND E", op_and, E, NULL, 4);
	SET_OP(0xA4, "AND H", op_and, H, NULL, 4);
	SET_OP(0xA5, "AND L", op_and, L, NULL, 4);
	SET_OP(0xA6, "AND (HL)", op_and, (HL), NULL, 8);
	SET_OP(0xA7, "AND A", op_and, A, NULL, 4);
	SET_OP(0xA8, "XOR B", op_xor, B, NULL, 4);
	SET_OP(0xA9, "XOR C", op_xor, C, NULL, 4);
	SET_OP(0xAA, "XOR D", op_xor, D, NULL, 4);
	SET_OP(0xAB, "XOR E", op_xor, E, NULL, 4);
	SET_OP(0xAC, "XOR H", op_xor, H, NULL, 4);
	SET_OP(0xAD, "XOR L", op_xor, L, NULL, 4);
	SET_OP(0xAE, "XOR (HL)", op_xor, (HL), NULL, 8);
	SET_OP(0xAF, "XOR A", op_xor, A, NULL, 4);
	
	SET_OP(0xB0, "OR B", op_or, B, NULL, 4);
	SET_OP(0xB1, "OR C", op_or, C, NULL, 4);
	SET_OP(0xB2, "OR D", op_or, D, NULL, 4);
	SET_OP(0xB3, "OR E", op_or, E, NULL, 4);
	SET_OP(0xB4, "OR H", op_or, H, NULL, 4);
	SET_OP(0xB5, "OR L", op_or, L, NULL, 4);
	SET_OP(0xB6, "OR (HL)", op_or, (HL), NULL, 8);
	SET_OP(0xB7, "OR A", op_or, A, NULL, 4);
	SET_OP(0xB8, "CP B", op_cp, B, NULL, 4);
	SET_OP(0xB9, "CP C", op_cp, C, NULL, 4);
	SET_OP(0xBA, "CP D", op_cp, D, NULL, 4);
	SET_OP(0xBB, "CP E", op_cp, E, NULL, 4);
	SET_OP(0xBC, "CP H", op_cp, H, NULL, 4);
	SET_OP(0xBD, "CP L", op_cp, L, NULL, 4);
	SET_OP(0xBE, "CP (HL)", op_cp, (HL), NULL, 8);
	SET_OP(0xBF, "CP A", op_cp, A, NULL, 4);

	SET_OP(0xC0, "", op_nop, NULL, NULL, 0);
	//...
	SET_OP(0xD0, "", op_nop, NULL, NULL, 0);
	//...
	
	SET_OP(0xE0, "LDH (a8),A", op_ld_8a_8r, pc1, A, 12);
	SET_OP(0xE1, "", op_nop, NULL, NULL, 0);
	SET_OP(0xE2, "LD (C),A", op_ld_8a_8r, C, A, 8);
	SET_OP(0xE3, "", op_nop, NULL, NULL, 0);
	SET_OP(0xE4, "", op_nop, NULL, NULL, 0);
	SET_OP(0xE5, "", op_nop, NULL, NULL, 0);
	SET_OP(0xE6, "", op_nop, NULL, NULL, 0);
	SET_OP(0xE7, "", op_nop, NULL, NULL, 0);
	SET_OP(0xE8, "", op_nop, NULL, NULL, 0);
	SET_OP(0xE9, "", op_nop, NULL, NULL, 0);
	SET_OP(0xEA, "LD (a16),A", op_ld_16a_8r, pc1, A, 16);
	SET_OP(0xEB, "", op_nop, NULL, NULL, 0);
	SET_OP(0xEC, "", op_nop, NULL, NULL, 0);
	SET_OP(0xED, "", op_nop, NULL, NULL, 0);
	SET_OP(0xEE, "", op_nop, NULL, NULL, 0);
	SET_OP(0xEF, "", op_nop, NULL, NULL, 0);
	
	SET_OP(0xF0, "LDH A,(a8)", op_ld_8r_8a, A, pc1, 12);
	SET_OP(0xF1, "", op_nop, NULL, NULL, 0);
	SET_OP(0xF2, "LD A,(C)", op_ld_8r_8a, A, C, 8);
	SET_OP(0xF3, "", op_nop, NULL, NULL, 0);
	SET_OP(0xF4, "", op_nop, NULL, NULL, 0);
	SET_OP(0xF5, "", op_nop, NULL, NULL, 0);
	SET_OP(0xF6, "", op_nop, NULL, NULL, 0);
	SET_OP(0xF7, "", op_nop, NULL, NULL, 0);
	SET_OP(0xF8, "LD HL,SP+r8", op_ld_16r_sp_8v, HL, pc1, 12);
	SET_OP(0xF9, "LD SP,HL", op_ld_16r_16r, SP, HL, 8);
	SET_OP(0xFA, "LD A,(a16)", op_ld_8r_16a, A, pc1, 16);
	SET_OP(0xFB, "", op_nop, NULL, NULL, 0);
	SET_OP(0xFC, "", op_nop, NULL, NULL, 0);
	SET_OP(0xFD, "", op_nop, NULL, NULL, 0);
	SET_OP(0xFE, "", op_nop, NULL, NULL, 0);
	SET_OP(0xFF, "", op_nop, NULL, NULL, 0);
}

void cpu_init() {
	set_opcodes();
	PC = 0;
	*F = 0x00;
}
