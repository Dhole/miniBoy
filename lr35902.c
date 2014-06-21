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

typedef enum {
	MEM_R_8,
	MEM_R_16,
	MEM_W_8,
	MEM_W_16,
	MEM_RW_16,
	NONE,
} opt_t;

typedef enum {
	COND_NZ = 0,
	COND_NC = 1,
	COND_Z = 2,
	COND_C = 3,
} cond_t;

// Some constants used as parameters
static uint8_t C_00H = 0x00, C_08H = 0x08, C_10H = 0x10, C_18H = 0x18,
	C_20H = 0x20, C_28H = 0x28, C_30H = 0x30, C_38H = 0x38;
static uint8_t C_0 = 0, C_1 = 1, C_2 = 2, C_3 = 3, C_4 = 4, C_5 = 5,
	C_6 = 6, C_7 = 7;
static uint8_t C_NZ = COND_NZ, C_NC = COND_NC, C_Z = COND_Z, C_C = COND_C;

static uint16_t imm_val;
static uint16_t *imm_16 = (uint16_t*)&imm_val;
static uint8_t *imm_8 = (uint8_t*)&imm_val;

static uint8_t tmp;
static uint8_t ext_cycles = 0;

typedef struct {
	char *desc;
	void (*handler)(void*, void*);
	void *a;
	void *b;
	opt_t opt;
	uint8_t length;
	uint8_t cycles[2];
} opcode_t;

static opcode_t ops[256];
static opcode_t ops_cb[256];

void set_flag(flag_t f, uint8_t v) {
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

void exec_op(uint8_t n) {
	printf("0x%04X : %02X -> %s\n", PC, n, ops[n].desc);
	
	switch (ops[n].length) {
	case 1: break;
	case 2: *imm_8 = mem_read_8(PC + 1); break;
	case 3: *imm_16 = mem_read_16(PC + 1); break;
	}
	PC += ops[n].length;

	switch (ops[n].opt) {
	case MEM_R_8:
		tmp = mem_read_8(*(uint8_t*)ops[n].b + 0xFF00);
		ops[n].handler(ops[n].a, &tmp);
		break;
	case MEM_R_16:
		tmp = mem_read_8(*(uint16_t*)ops[n].b);
		ops[n].handler(ops[n].a, &tmp);
		break;
	case MEM_W_8:
		ops[n].handler(&tmp, ops[n].b);
		mem_write_8(*(uint8_t*)ops[n].a + 0xFF00, tmp);
		break;
	case MEM_W_16:
		ops[n].handler(&tmp, ops[n].b);
		mem_write_8(*(uint16_t*)ops[n].a, tmp);
		break;
	case NONE:
		ops[n].handler(ops[n].a, ops[n].b);
		break;
	}
}

void exec_op_cb(uint8_t n) {
	printf("0x%04X : %02X -> %s\n", PC, n, ops_cb[n].desc);
	
	switch (ops_cb[n].opt) {
	case MEM_R_16:
		tmp = mem_read_8(*(uint16_t*)ops_cb[n].b);
		ops[n].handler(ops_cb[n].a, &tmp);
		break;
	case MEM_W_16:
		ops_cb[n].handler(&tmp, ops_cb[n].b);
		mem_write_8(*(uint16_t*)ops_cb[n].a, tmp);
		break;
	case MEM_RW_16:
		tmp = mem_read_8(*(uint16_t*)ops_cb[n].a);
		ops[n].handler(&tmp, ops_cb[n].b);
		mem_write_8(*(uint16_t*)ops_cb[n].a, tmp);
		
	case NONE:
		ops_cb[n].handler(ops[n].a, ops[n].b);
		break;
	}
}

int cpu_step() {
	uint8_t op;
	//uint8_t d8, a8, r8;
	//uint16_t d16, a16;

	op = mem_read_8(PC);
	
	//OP(op);
	exec_op(op);

	if (op == 0xCB) {
		return ops_cb[mem_read_8(PC + 1)].cycles[0];
	} else {
		if (ops[op].cycles[1]) {
			if (ext_cycles) {
				ext_cycles = 0;
				return ops[op].cycles[1];
			} else {
				return ops[op].cycles[0];
			}
		} else {
			return ops[op].cycles[0];
		}
	}
	
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
	printf("Z N H C\n");
	printf("%d %d %d %d\n", (*F & Z_FLAG) >> 7, (*F & N_FLAG) >> 6,
	       (*F & H_FLAG) >> 5, (*F & C_FLAG) >> 4);
}

void cpu_test() {
	char t;
	
	while (1) {
		printf("> ");
		scanf("%c%*c", &t);
		switch (t) {
		case 's':
			cpu_step();
			break;
		case 'r':
			cpu_dump_reg();
			break;
		case 'm':
			mem_dump(0xFFF0, 0xFFFF);
			break;
		default:
			cpu_step();
			break;
		}
	}
}

void op_ld_8(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a = *b;
}

void op_ld_16(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	*a = *b;
}

void op_ldi(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a = *b;
	*HL++;
}

void op_ldd(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a = *b;
	*HL--;
}

void op_ldhl(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	int8_t *b = (int8_t*)_b;
	*HL = *a + *b;
	
	set_flag(Z_FLAG, 0);
	set_flag(N_FLAG, 0);
	// Not sure if this is right
	set_flag(H_FLAG, (*a & 0x0F > 0x0F - *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a & 0xFF > 0xFF - *b) ? 1 : 0);
}

void op_halt(void *_a, void *_b) {
	// TODO
}

void op_add_8(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a += *b;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0F > 0x0F - *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a > 0xFF - *b) ? 1 : 0);
}

void op_add_16(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	*a += *b;

	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0FFF > 0x0FFF - *b & 0x0FFF) ? 1 : 0);
	set_flag(C_FLAG, (*a & 0xFFFF > 0xFFFF - *b & 0xFFFF) ? 1 : 0);
}

void op_addsp(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	int8_t *b = (int8_t*)_b;
	*a += *b;

	set_flag(Z_FLAG, 0);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0F > 0x0F - *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a & 0xFF > 0xFF - *b) ? 1 : 0);
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
	set_flag(H_FLAG, (*a & 0x0F < *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a < *b) ? 1 : 0);
}

void op_sbc(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a -= *b + get_flag(C_FLAG);

        set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0F < *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a < *b) ? 1 : 0);
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
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	
	set_flag(Z_FLAG, (*a == *b) ? 1 : 0);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a & 0x0F > 0x0F - *b & 0x0F) ? 1 : 0);
	set_flag(C_FLAG, (*a < *b) ? 1 : 0);
}

void op_inc_8(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	*a++;
	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a == 0x00) ? 1 : 0);
}

void op_inc_16(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	*a++;
}

void op_dec_8(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	*a--;
	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, (*a == 0xFF) ? 1 : 0);
}

void op_dec_16(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	*a--;
}

void op_ccf(void *_a, void *_b) {
	if (*F & C_FLAG) {
		set_flag(C_FLAG, 0);
	} else {
		set_flag(C_FLAG, 1);
	}
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
}

void op_scf(void *_a, void *_b) {
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, 1);
}

void op_rlca(void *_a, void *_b) {
	uint8_t b7 = (*A & 0x80) >> 7;
	*A = *A << 1;
	*A += b7;

	set_flag_Z(A);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b7);
}

void op_rla(void *_a, void *_b) {
	uint8_t b7 = (*A & 0x80) >> 7;
	*A = *A << 1;
	*A += (*F & C_FLAG) >> 4;

	set_flag_Z(A);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b7);
}


void op_rrca(void *_a, void *_b) {
	uint8_t b0 = *A & 0x01;
	*A = *A >> 1;
	*A += b0 << 7;

	set_flag_Z(A);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b0);
}

void op_rra(void *_a, void *_b) {
        uint8_t b0 = *A & 0x01;
	*A = *A >> 1;
	*A += ((*F & C_FLAG) >> 4) << 7;

	set_flag_Z(A);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b0);
}

void op_di(void *_a, void *_b) {
        // TODO
	// Disable interrupts
}

void op_ei(void *_a, void *_b) {
        // TODO
	// Enable interrupts
}

void op_push(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	mem_write_16(*SP, *a);
	*SP -= 2;
}

void op_pop(void *_a, void *_b) {
	uint16_t *a = (uint16_t*)_a;
	*SP += 2;
	*a = mem_read_16(*SP);
}

void op_jp(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	uint8_t jump;
	if (a == NULL) {
		PC = *b;
	} else {
		switch (*a) {
		case COND_NZ: jump = (*F & Z_FLAG) ? 0 : 1; break;
		case COND_Z: jump = (*F & Z_FLAG) ? 1 : 0; break;
		case COND_NC: jump = (*F & C_FLAG) ? 0 : 1; break;
		case COND_C: jump = (*F & C_FLAG) ? 1 : 0; break;
		}
		if (jump) {
			PC = *b;
			ext_cycles = 1;
		}
	}
}

void op_jr(void *_a, void *_b) {
        uint8_t *a = (uint8_t*)_a;
	int8_t *b = (int8_t*)_b;
	uint8_t jump;
	if (a == NULL) {
		PC += *b;
	} else {
		switch (*a) {
		case COND_NZ: jump = (*F & Z_FLAG) ? 0 : 1; break;
		case COND_Z: jump = (*F & Z_FLAG) ? 1 : 0; break;
		case COND_NC: jump = (*F & C_FLAG) ? 0 : 1; break;
		case COND_C: jump = (*F & C_FLAG) ? 1 : 0; break;
		}
		if (jump) {
			PC += *b;
			ext_cycles = 1;
		}
	}
}


void op_call(void *_a, void *_b) {
        uint8_t *a = (uint8_t*)_a;
	uint16_t *b = (uint16_t*)_b;
	uint8_t jump;
	if (a == NULL) {
		op_push(&PC, NULL);
		PC = *b;
	} else {
		switch (*a) {
		case COND_NZ: jump = (*F & Z_FLAG) ? 0 : 1; break;
		case COND_Z: jump = (*F & Z_FLAG) ? 1 : 0; break;
		case COND_NC: jump = (*F & C_FLAG) ? 0 : 1; break;
		case COND_C: jump = (*F & C_FLAG) ? 1 : 0; break;
		}
		if (jump) {
			op_push(&PC, NULL);
			PC = *b;
			ext_cycles = 1;
		}
	}
}

void op_ret(void *_a, void *_b) {
        uint8_t *a = (uint8_t*)_a;
	uint8_t jump;
	if (a == NULL) {
		op_pop(&PC, NULL);
	} else {
		switch (*a) {
		case COND_NZ: jump = (*F & Z_FLAG) ? 0 : 1; break;
		case COND_Z: jump = (*F & Z_FLAG) ? 1 : 0; break;
		case COND_NC: jump = (*F & C_FLAG) ? 0 : 1; break;
		case COND_C: jump = (*F & C_FLAG) ? 1 : 0; break;
		}
		if (jump) {
			op_pop(&PC, NULL);
			ext_cycles = 1;
		}
	}
}

void op_reti(void *_a, void *_b) {
	// TODO
        op_pop(&PC, NULL);
	op_ei(NULL, NULL);
}

void op_rst(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
        op_push(&PC, NULL);
	PC = *a;
}

void op_cpl(void *_a, void *_b) {
	*A = ~*A;
	set_flag(N_FLAG, 1);
	set_flag(H_FLAG, 1);
}

void op_daa(void *_a, void *_b) {
	// TODO
        //set_flag_Z(a);
	//set_flag(H_FLAG, 0);
	//set_flag(C_FLAG, 0);
}

void op_stop(void *_a, void *_b) {
        // TODO
}

void op_nop(void *_a, void *_b) {
        
}

void op_undef(void *_a, void *_b) {
        // TODO
}

void op_pref_cb(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	exec_op_cb(*a);
}

// prefix CB opcodes
void op_rlc(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t b7 = (*a & 0x80) >> 7;
	*a = *a << 1;
	*a += b7;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b7);
}

void op_rrc(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t b0 = *a & 0x01;
	*a = *a >> 1;
	*a += b0 << 7;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b0);
}

void op_rl(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t b7 = (*a & 0x80) >> 7;
	*a = *a << 1;
	*a += (*F & C_FLAG) >> 4;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b7);
}

void op_rr(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t b0 = *a & 0x01;
	*a = *a >> 1;
	*a += ((*F & C_FLAG) >> 4) << 7;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b0);
}

void op_sla(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t b7 = (*a & 0x80) >> 7;
	*a = *a << 1;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b7);
}

void op_sra(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t b0 = *a & 0x01;
	uint8_t b7 = (*a & 0x80) >> 7;
	*a = *a >> 1;
	*a += b7 << 7;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b0);
}

void op_swap(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	*a = (*a << 4) + (*a & 0xF0 >> 4);
}

void op_srl(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t b0 = *a & 0x01;
	*a = *a >> 1;

	set_flag_Z(a);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 0);
	set_flag(C_FLAG, b0);
}

void op_bit(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	
	set_flag(Z_FLAG, (*a & (1 << *b)) ? 0 : 1);
	set_flag(N_FLAG, 0);
	set_flag(H_FLAG, 1);
}

void op_res(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a &= ~(1 << *b);
}

void op_set(void *_a, void *_b) {
	uint8_t *a = (uint8_t*)_a;
	uint8_t *b = (uint8_t*)_b;
	*a |= 1 << *b;
}

/*
#define SET_OP(i, d, f, _a, _b, c)		\
	ops[i].desc = d;			\
	ops[i].handler = &f;			\
	ops[i].a = _a;				\
	ops[i].b = _b;				\
	ops[i].cycles = c;
*/

// (opcode, description, function handler, parameter a, parameter b,
//  memory access, byte length, normal cycles, extra cycles)
#define SET_OP(i, d, f, _a, _b, o, l, c0, c1)	\
	ops[i].desc = d;			\
	ops[i].handler = &f;			\
	ops[i].a = _a;				\
	ops[i].b = _b;				\
	ops[i].opt = o;				\
	ops[i].length = l;			\
	ops[i].cycles[0] = c0;			\
	ops[i].cycles[1] = c1;

void set_opcodes() {
SET_OP(0x00, "NOP", op_nop, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x01, "LD BC,d16", op_ld_16, BC, imm_16, NONE, 3, 12, 0);
SET_OP(0x02, "LD (BC),A", op_ld_8, BC, A, MEM_W_16, 1, 8, 0);
SET_OP(0x03, "INC BC", op_inc_16, BC, NULL, NONE, 1, 8, 0);
SET_OP(0x04, "INC B", op_inc_8, B, NULL, NONE, 1, 4, 0);
SET_OP(0x05, "DEC B", op_dec_8, B, NULL, NONE, 1, 4, 0);
SET_OP(0x06, "LD B,d8", op_ld_8, B, imm_8, NONE, 2, 8, 0);
SET_OP(0x07, "RLCA", op_rlca, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x08, "LD (a16),SP", op_ld_16, imm_16, SP, MEM_W_16, 3, 20, 0);
SET_OP(0x09, "ADD HL,BC", op_add_16, HL, BC, NONE, 1, 8, 0);
SET_OP(0x0A, "LD A,(BC)", op_ld_8, A, BC, MEM_R_16, 1, 8, 0);
SET_OP(0x0B, "DEC BC", op_dec_16, BC, NULL, NONE, 1, 8, 0);
SET_OP(0x0C, "INC C", op_inc_8, C, NULL, NONE, 1, 4, 0);
SET_OP(0x0D, "DEC C", op_dec_8, C, NULL, NONE, 1, 4, 0);
SET_OP(0x0E, "LD C,d8", op_ld_8, C, imm_8, NONE, 2, 8, 0);
SET_OP(0x0F, "RRCA", op_rrca, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x10, "STOP 0", op_stop, &C_0, NULL, NONE, 2, 4, 0);
SET_OP(0x11, "LD DE,d16", op_ld_16, DE, imm_16, NONE, 3, 12, 0);
SET_OP(0x12, "LD (DE),A", op_ld_8, DE, A, MEM_W_16, 1, 8, 0);
SET_OP(0x13, "INC DE", op_inc_16, DE, NULL, NONE, 1, 8, 0);
SET_OP(0x14, "INC D", op_inc_8, D, NULL, NONE, 1, 4, 0);
SET_OP(0x15, "DEC D", op_dec_8, D, NULL, NONE, 1, 4, 0);
SET_OP(0x16, "LD D,d8", op_ld_8, D, imm_8, NONE, 2, 8, 0);
SET_OP(0x17, "RLA", op_rla, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x18, "JR r8", op_jr, NULL, imm_8, NONE, 2, 12, 0);
SET_OP(0x19, "ADD HL,DE", op_add_16, HL, DE, NONE, 1, 8, 0);
SET_OP(0x1A, "LD A,(DE)", op_ld_8, A, DE, MEM_R_16, 1, 8, 0);
SET_OP(0x1B, "DEC DE", op_dec_16, DE, NULL, NONE, 1, 8, 0);
SET_OP(0x1C, "INC E", op_inc_8, E, NULL, NONE, 1, 4, 0);
SET_OP(0x1D, "DEC E", op_dec_8, E, NULL, NONE, 1, 4, 0);
SET_OP(0x1E, "LD E,d8", op_ld_8, E, imm_8, NONE, 2, 8, 0);
SET_OP(0x1F, "RRA", op_rra, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x20, "JR NZ,r8", op_jr, &C_NZ, imm_8, NONE, 2, 8, 12);
SET_OP(0x21, "LD HL,d16", op_ld_16, HL, imm_16, NONE, 3, 12, 0);
SET_OP(0x22, "LD (HL+),A", op_ldi, HL, A, MEM_W_16, 1, 8, 0);
SET_OP(0x23, "INC HL", op_inc_16, HL, NULL, NONE, 1, 8, 0);
SET_OP(0x24, "INC H", op_inc_8, H, NULL, NONE, 1, 4, 0);
SET_OP(0x25, "DEC H", op_dec_8, H, NULL, NONE, 1, 4, 0);
SET_OP(0x26, "LD H,d8", op_ld_8, H, imm_8, NONE, 2, 8, 0);
SET_OP(0x27, "DAA", op_daa, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x28, "JR Z,r8", op_jr, &C_Z, imm_8, NONE, 2, 8, 12);
SET_OP(0x29, "ADD HL,HL", op_add_16, HL, HL, NONE, 1, 8, 0);
SET_OP(0x2A, "LD A,(HL+)", op_ldi, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x2B, "DEC HL", op_dec_16, HL, NULL, NONE, 1, 8, 0);
SET_OP(0x2C, "INC L", op_inc_8, L, NULL, NONE, 1, 4, 0);
SET_OP(0x2D, "DEC L", op_dec_8, L, NULL, NONE, 1, 4, 0);
SET_OP(0x2E, "LD L,d8", op_ld_8, L, imm_8, NONE, 2, 8, 0);
SET_OP(0x2F, "CPL", op_cpl, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x30, "JR NC,r8", op_jr, &C_NC, imm_8, NONE, 2, 8, 12);
SET_OP(0x31, "LD SP,d16", op_ld_16, SP, imm_16, NONE, 3, 12, 0);
SET_OP(0x32, "LD (HL-),A", op_ldd, HL, A, MEM_W_16, 1, 8, 0);
SET_OP(0x33, "INC SP", op_inc_16, SP, NULL, NONE, 1, 8, 0);
SET_OP(0x34, "INC (HL)", op_inc_8, HL, NULL, MEM_W_16, 1, 12, 0);
SET_OP(0x35, "DEC (HL)", op_dec_8, HL, NULL, MEM_W_16, 1, 12, 0);
SET_OP(0x36, "LD (HL),d8", op_ld_8, HL, imm_8, MEM_W_16, 2, 12, 0);
SET_OP(0x37, "SCF", op_scf, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x38, "JR C,r8", op_jr, &C_C, imm_8, NONE, 2, 8, 12);
SET_OP(0x39, "ADD HL,SP", op_add_16, HL, SP, NONE, 1, 8, 0);
SET_OP(0x3A, "LD A,(HL-)", op_ldd, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x3B, "DEC SP", op_dec_16, SP, NULL, NONE, 1, 8, 0);
SET_OP(0x3C, "INC A", op_inc_8, A, NULL, NONE, 1, 4, 0);
SET_OP(0x3D, "DEC A", op_dec_8, A, NULL, NONE, 1, 4, 0);
SET_OP(0x3E, "LD A,d8", op_ld_8, A, imm_8, NONE, 2, 8, 0);
SET_OP(0x3F, "CCF", op_ccf, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x40, "LD B,B", op_ld_8, B, B, NONE, 1, 4, 0);
SET_OP(0x41, "LD B,C", op_ld_8, B, C, NONE, 1, 4, 0);
SET_OP(0x42, "LD B,D", op_ld_8, B, D, NONE, 1, 4, 0);
SET_OP(0x43, "LD B,E", op_ld_8, B, E, NONE, 1, 4, 0);
SET_OP(0x44, "LD B,H", op_ld_8, B, H, NONE, 1, 4, 0);
SET_OP(0x45, "LD B,L", op_ld_8, B, L, NONE, 1, 4, 0);
SET_OP(0x46, "LD B,(HL)", op_ld_8, B, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x47, "LD B,A", op_ld_8, B, A, NONE, 1, 4, 0);
SET_OP(0x48, "LD C,B", op_ld_8, C, B, NONE, 1, 4, 0);
SET_OP(0x49, "LD C,C", op_ld_8, C, C, NONE, 1, 4, 0);
SET_OP(0x4A, "LD C,D", op_ld_8, C, D, NONE, 1, 4, 0);
SET_OP(0x4B, "LD C,E", op_ld_8, C, E, NONE, 1, 4, 0);
SET_OP(0x4C, "LD C,H", op_ld_8, C, H, NONE, 1, 4, 0);
SET_OP(0x4D, "LD C,L", op_ld_8, C, L, NONE, 1, 4, 0);
SET_OP(0x4E, "LD C,(HL)", op_ld_8, C, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x4F, "LD C,A", op_ld_8, C, A, NONE, 1, 4, 0);
SET_OP(0x50, "LD D,B", op_ld_8, D, B, NONE, 1, 4, 0);
SET_OP(0x51, "LD D,C", op_ld_8, D, C, NONE, 1, 4, 0);
SET_OP(0x52, "LD D,D", op_ld_8, D, D, NONE, 1, 4, 0);
SET_OP(0x53, "LD D,E", op_ld_8, D, E, NONE, 1, 4, 0);
SET_OP(0x54, "LD D,H", op_ld_8, D, H, NONE, 1, 4, 0);
SET_OP(0x55, "LD D,L", op_ld_8, D, L, NONE, 1, 4, 0);
SET_OP(0x56, "LD D,(HL)", op_ld_8, D, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x57, "LD D,A", op_ld_8, D, A, NONE, 1, 4, 0);
SET_OP(0x58, "LD E,B", op_ld_8, E, B, NONE, 1, 4, 0);
SET_OP(0x59, "LD E,C", op_ld_8, E, C, NONE, 1, 4, 0);
SET_OP(0x5A, "LD E,D", op_ld_8, E, D, NONE, 1, 4, 0);
SET_OP(0x5B, "LD E,E", op_ld_8, E, E, NONE, 1, 4, 0);
SET_OP(0x5C, "LD E,H", op_ld_8, E, H, NONE, 1, 4, 0);
SET_OP(0x5D, "LD E,L", op_ld_8, E, L, NONE, 1, 4, 0);
SET_OP(0x5E, "LD E,(HL)", op_ld_8, E, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x5F, "LD E,A", op_ld_8, E, A, NONE, 1, 4, 0);
SET_OP(0x60, "LD H,B", op_ld_8, H, B, NONE, 1, 4, 0);
SET_OP(0x61, "LD H,C", op_ld_8, H, C, NONE, 1, 4, 0);
SET_OP(0x62, "LD H,D", op_ld_8, H, D, NONE, 1, 4, 0);
SET_OP(0x63, "LD H,E", op_ld_8, H, E, NONE, 1, 4, 0);
SET_OP(0x64, "LD H,H", op_ld_8, H, H, NONE, 1, 4, 0);
SET_OP(0x65, "LD H,L", op_ld_8, H, L, NONE, 1, 4, 0);
SET_OP(0x66, "LD H,(HL)", op_ld_8, H, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x67, "LD H,A", op_ld_8, H, A, NONE, 1, 4, 0);
SET_OP(0x68, "LD L,B", op_ld_8, L, B, NONE, 1, 4, 0);
SET_OP(0x69, "LD L,C", op_ld_8, L, C, NONE, 1, 4, 0);
SET_OP(0x6A, "LD L,D", op_ld_8, L, D, NONE, 1, 4, 0);
SET_OP(0x6B, "LD L,E", op_ld_8, L, E, NONE, 1, 4, 0);
SET_OP(0x6C, "LD L,H", op_ld_8, L, H, NONE, 1, 4, 0);
SET_OP(0x6D, "LD L,L", op_ld_8, L, L, NONE, 1, 4, 0);
SET_OP(0x6E, "LD L,(HL)", op_ld_8, L, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x6F, "LD L,A", op_ld_8, L, A, NONE, 1, 4, 0);
SET_OP(0x70, "LD (HL),B", op_ld_8, HL, B, MEM_W_16, 1, 8, 0);
SET_OP(0x71, "LD (HL),C", op_ld_8, HL, C, MEM_W_16, 1, 8, 0);
SET_OP(0x72, "LD (HL),D", op_ld_8, HL, D, MEM_W_16, 1, 8, 0);
SET_OP(0x73, "LD (HL),E", op_ld_8, HL, E, MEM_W_16, 1, 8, 0);
SET_OP(0x74, "LD (HL),H", op_ld_8, HL, H, MEM_W_16, 1, 8, 0);
SET_OP(0x75, "LD (HL),L", op_ld_8, HL, L, MEM_W_16, 1, 8, 0);
SET_OP(0x76, "HALT", op_halt, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0x77, "LD (HL),A", op_ld_8, HL, A, MEM_W_16, 1, 8, 0);
SET_OP(0x78, "LD A,B", op_ld_8, A, B, NONE, 1, 4, 0);
SET_OP(0x79, "LD A,C", op_ld_8, A, C, NONE, 1, 4, 0);
SET_OP(0x7A, "LD A,D", op_ld_8, A, D, NONE, 1, 4, 0);
SET_OP(0x7B, "LD A,E", op_ld_8, A, E, NONE, 1, 4, 0);
SET_OP(0x7C, "LD A,H", op_ld_8, A, H, NONE, 1, 4, 0);
SET_OP(0x7D, "LD A,L", op_ld_8, A, L, NONE, 1, 4, 0);
SET_OP(0x7E, "LD A,(HL)", op_ld_8, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x7F, "LD A,A", op_ld_8, A, A, NONE, 1, 4, 0);
SET_OP(0x80, "ADD A,B", op_add_8, A, B, NONE, 1, 4, 0);
SET_OP(0x81, "ADD A,C", op_add_8, A, C, NONE, 1, 4, 0);
SET_OP(0x82, "ADD A,D", op_add_8, A, D, NONE, 1, 4, 0);
SET_OP(0x83, "ADD A,E", op_add_8, A, E, NONE, 1, 4, 0);
SET_OP(0x84, "ADD A,H", op_add_8, A, H, NONE, 1, 4, 0);
SET_OP(0x85, "ADD A,L", op_add_8, A, L, NONE, 1, 4, 0);
SET_OP(0x86, "ADD A,(HL)", op_add_8, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x87, "ADD A,A", op_add_8, A, A, NONE, 1, 4, 0);
SET_OP(0x88, "ADC A,B", op_adc, A, B, NONE, 1, 4, 0);
SET_OP(0x89, "ADC A,C", op_adc, A, C, NONE, 1, 4, 0);
SET_OP(0x8A, "ADC A,D", op_adc, A, D, NONE, 1, 4, 0);
SET_OP(0x8B, "ADC A,E", op_adc, A, E, NONE, 1, 4, 0);
SET_OP(0x8C, "ADC A,H", op_adc, A, H, NONE, 1, 4, 0);
SET_OP(0x8D, "ADC A,L", op_adc, A, L, NONE, 1, 4, 0);
SET_OP(0x8E, "ADC A,(HL)", op_adc, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x8F, "ADC A,A", op_adc, A, A, NONE, 1, 4, 0);
SET_OP(0x90, "SUB B", op_sub, A, B, NONE, 1, 4, 0);
SET_OP(0x91, "SUB C", op_sub, A, C, NONE, 1, 4, 0);
SET_OP(0x92, "SUB D", op_sub, A, D, NONE, 1, 4, 0);
SET_OP(0x93, "SUB E", op_sub, A, E, NONE, 1, 4, 0);
SET_OP(0x94, "SUB H", op_sub, A, H, NONE, 1, 4, 0);
SET_OP(0x95, "SUB L", op_sub, A, L, NONE, 1, 4, 0);
SET_OP(0x96, "SUB (HL)", op_sub, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x97, "SUB A", op_sub, A, A, NONE, 1, 4, 0);
SET_OP(0x98, "SBC A,B", op_sbc, A, B, NONE, 1, 4, 0);
SET_OP(0x99, "SBC A,C", op_sbc, A, C, NONE, 1, 4, 0);
SET_OP(0x9A, "SBC A,D", op_sbc, A, D, NONE, 1, 4, 0);
SET_OP(0x9B, "SBC A,E", op_sbc, A, E, NONE, 1, 4, 0);
SET_OP(0x9C, "SBC A,H", op_sbc, A, H, NONE, 1, 4, 0);
SET_OP(0x9D, "SBC A,L", op_sbc, A, L, NONE, 1, 4, 0);
SET_OP(0x9E, "SBC A,(HL)", op_sbc, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0x9F, "SBC A,A", op_sbc, A, A, NONE, 1, 4, 0);
SET_OP(0xA0, "AND B", op_and, A, B, NONE, 1, 4, 0);
SET_OP(0xA1, "AND C", op_and, A, C, NONE, 1, 4, 0);
SET_OP(0xA2, "AND D", op_and, A, D, NONE, 1, 4, 0);
SET_OP(0xA3, "AND E", op_and, A, E, NONE, 1, 4, 0);
SET_OP(0xA4, "AND H", op_and, A, H, NONE, 1, 4, 0);
SET_OP(0xA5, "AND L", op_and, A, L, NONE, 1, 4, 0);
SET_OP(0xA6, "AND (HL)", op_and, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0xA7, "AND A", op_and, A, A, NONE, 1, 4, 0);
SET_OP(0xA8, "XOR B", op_xor, A, B, NONE, 1, 4, 0);
SET_OP(0xA9, "XOR C", op_xor, A, C, NONE, 1, 4, 0);
SET_OP(0xAA, "XOR D", op_xor, A, D, NONE, 1, 4, 0);
SET_OP(0xAB, "XOR E", op_xor, A, E, NONE, 1, 4, 0);
SET_OP(0xAC, "XOR H", op_xor, A, H, NONE, 1, 4, 0);
SET_OP(0xAD, "XOR L", op_xor, A, L, NONE, 1, 4, 0);
SET_OP(0xAE, "XOR (HL)", op_xor, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0xAF, "XOR A", op_xor, A, A, NONE, 1, 4, 0);
SET_OP(0xB0, "OR B", op_or, A, B, NONE, 1, 4, 0);
SET_OP(0xB1, "OR C", op_or, A, C, NONE, 1, 4, 0);
SET_OP(0xB2, "OR D", op_or, A, D, NONE, 1, 4, 0);
SET_OP(0xB3, "OR E", op_or, A, E, NONE, 1, 4, 0);
SET_OP(0xB4, "OR H", op_or, A, H, NONE, 1, 4, 0);
SET_OP(0xB5, "OR L", op_or, A, L, NONE, 1, 4, 0);
SET_OP(0xB6, "OR (HL)", op_or, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0xB7, "OR A", op_or, A, A, NONE, 1, 4, 0);
SET_OP(0xB8, "CP B", op_cp, A, B, NONE, 1, 4, 0);
SET_OP(0xB9, "CP C", op_cp, A, C, NONE, 1, 4, 0);
SET_OP(0xBA, "CP D", op_cp, A, D, NONE, 1, 4, 0);
SET_OP(0xBB, "CP E", op_cp, A, E, NONE, 1, 4, 0);
SET_OP(0xBC, "CP H", op_cp, A, H, NONE, 1, 4, 0);
SET_OP(0xBD, "CP L", op_cp, A, L, NONE, 1, 4, 0);
SET_OP(0xBE, "CP (HL)", op_cp, A, HL, MEM_R_16, 1, 8, 0);
SET_OP(0xBF, "CP A", op_cp, A, A, NONE, 1, 4, 0);
SET_OP(0xC0, "RET NZ", op_ret, &C_NZ, NULL, NONE, 1, 8, 20);
SET_OP(0xC1, "POP BC", op_pop, BC, NULL, NONE, 1, 12, 0);
SET_OP(0xC2, "JP NZ,a16", op_jp, &C_NZ, imm_16, NONE, 3, 12, 16);
SET_OP(0xC3, "JP a16", op_jp, NULL, imm_16, NONE, 3, 16, 0);
SET_OP(0xC4, "CALL NZ,a16", op_call, &C_NZ, imm_16, NONE, 3, 12, 24);
SET_OP(0xC5, "PUSH BC", op_push, BC, NULL, NONE, 1, 16, 0);
SET_OP(0xC6, "ADD A,d8", op_add_8, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xC7, "RST 00H", op_rst, &C_00H, NULL, NONE, 1, 16, 0);
SET_OP(0xC8, "RET Z", op_ret, &C_Z, NULL, NONE, 1, 8, 20);
SET_OP(0xC9, "RET", op_ret, NULL, NULL, NONE, 1, 16, 0);
SET_OP(0xCA, "JP Z,a16", op_jp, &C_Z, imm_16, NONE, 3, 12, 16);
SET_OP(0xCB, "PREFIX CB", op_pref_cb, imm_8, NULL, NONE, 2, 4, 0);
SET_OP(0xCC, "CALL Z,a16", op_call, &C_Z, imm_16, NONE, 3, 12, 24);
SET_OP(0xCD, "CALL a16", op_call, NULL, imm_16, NONE, 3, 24, 0);
SET_OP(0xCE, "ADC A,d8", op_adc, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xCF, "RST 08H", op_rst, &C_08H, NULL, NONE, 1, 16, 0);
SET_OP(0xD0, "RET NC", op_ret, &C_NC, NULL, NONE, 1, 8, 20);
SET_OP(0xD1, "POP DE", op_pop, DE, NULL, NONE, 1, 12, 0);
SET_OP(0xD2, "JP NC,a16", op_jp, &C_NC, imm_16, NONE, 3, 12, 16);
SET_OP(0xD3, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xD4, "CALL NC,a16", op_call, &C_NC, imm_16, NONE, 3, 12, 24);
SET_OP(0xD5, "PUSH DE", op_push, DE, NULL, NONE, 1, 16, 0);
SET_OP(0xD6, "SUB d8", op_sub, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xD7, "RST 10H", op_rst, &C_10H, NULL, NONE, 1, 16, 0);
SET_OP(0xD8, "RET C", op_ret, &C_C, NULL, NONE, 1, 8, 20);
SET_OP(0xD9, "RETI", op_reti, NULL, NULL, NONE, 1, 16, 0);
SET_OP(0xDA, "JP C,a16", op_jp, &C_C, imm_16, NONE, 3, 12, 16);
SET_OP(0xDB, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xDC, "CALL C,a16", op_call, &C_C, imm_16, NONE, 3, 12, 24);
SET_OP(0xDD, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xDE, "SBC A,d8", op_sbc, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xDF, "RST 18H", op_rst, &C_18H, NULL, NONE, 1, 16, 0);
SET_OP(0xE0, "LDH (a8),A", op_ld_8, imm_8, A, MEM_W_8, 2, 12, 0);
SET_OP(0xE1, "POP HL", op_pop, HL, NULL, NONE, 1, 12, 0);
SET_OP(0xE2, "LD (C),A", op_ld_8, C, A, MEM_W_8, 1, 8, 0);
SET_OP(0xE3, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xE4, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xE5, "PUSH HL", op_push, HL, NULL, NONE, 1, 16, 0);
SET_OP(0xE6, "AND d8", op_and, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xE7, "RST 20H", op_rst, &C_20H, NULL, NONE, 1, 16, 0);
SET_OP(0xE8, "ADD SP,r8", op_addsp, SP, imm_8, NONE, 2, 16, 0);
SET_OP(0xE9, "JP (HL)", op_jp, NULL, HL, MEM_R_16, 1, 4, 0);
SET_OP(0xEA, "LD (a16),A", op_ld_8, imm_16, A, MEM_W_16, 3, 16, 0);
SET_OP(0xEB, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xEC, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xED, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xEE, "XOR d8", op_xor, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xEF, "RST 28H", op_rst, &C_28H, NULL, NONE, 1, 16, 0);
SET_OP(0xF0, "LDH A,(a8)", op_ld_8, A, imm_8, MEM_R_8, 2, 12, 0);
SET_OP(0xF1, "POP AF", op_pop, AF, NULL, NONE, 1, 12, 0);
SET_OP(0xF2, "LD A,(C)", op_ld_8, A, C, MEM_R_8, 1, 8, 0);
SET_OP(0xF3, "DI", op_di, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0xF4, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xF5, "PUSH AF", op_push, AF, NULL, NONE, 1, 16, 0);
SET_OP(0xF6, "OR d8", op_or, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xF7, "RST 30H", op_rst, &C_30H, NULL, NONE, 1, 16, 0);
SET_OP(0xF8, "LD HL,SP+r8", op_ldhl, SP, imm_8, NONE, 2, 12, 0);
SET_OP(0xF9, "LD SP,HL", op_ld_16, SP, HL, NONE, 1, 8, 0);
SET_OP(0xFA, "LD A,(a16)", op_ld_8, A, imm_16, MEM_R_16, 3, 16, 0);
SET_OP(0xFB, "EI", op_ei, NULL, NULL, NONE, 1, 4, 0);
SET_OP(0xFC, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xFD, "-", op_undef, NULL, NULL, NONE, 1, 0, 0);
SET_OP(0xFE, "CP d8", op_cp, A, imm_8, NONE, 2, 8, 0);
SET_OP(0xFF, "RST 38H", op_rst, &C_38H, NULL, NONE, 1, 16, 0);
}

#define SET_OP_CB(i, d, f, _a, _b, o, l, c0, c1)	\
	ops_cb[i].desc = d;				\
	ops_cb[i].handler = &f;				\
	ops_cb[i].a = _a;				\
	ops_cb[i].b = _b;				\
	ops_cb[i].opt = o;				\
	ops_cb[i].length = l;				\
	ops_cb[i].cycles[0] = c0;			\
	ops_cb[i].cycles[1] = c1;

void set_opcodes_cb() {
	SET_OP_CB(0x00, "RLC B", op_rlc, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x01, "RLC C", op_rlc, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x02, "RLC D", op_rlc, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x03, "RLC E", op_rlc, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x04, "RLC H", op_rlc, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x05, "RLC L", op_rlc, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x06, "RLC (HL)", op_rlc, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x07, "RLC A", op_rlc, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x08, "RRC B", op_rrc, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x09, "RRC C", op_rrc, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x0A, "RRC D", op_rrc, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x0B, "RRC E", op_rrc, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x0C, "RRC H", op_rrc, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x0D, "RRC L", op_rrc, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x0E, "RRC (HL)", op_rrc, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x0F, "RRC A", op_rrc, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x10, "RL B", op_rl, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x11, "RL C", op_rl, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x12, "RL D", op_rl, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x13, "RL E", op_rl, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x14, "RL H", op_rl, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x15, "RL L", op_rl, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x16, "RL (HL)", op_rl, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x17, "RL A", op_rl, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x18, "RR B", op_rr, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x19, "RR C", op_rr, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x1A, "RR D", op_rr, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x1B, "RR E", op_rr, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x1C, "RR H", op_rr, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x1D, "RR L", op_rr, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x1E, "RR (HL)", op_rr, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x1F, "RR A", op_rr, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x20, "SLA B", op_sla, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x21, "SLA C", op_sla, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x22, "SLA D", op_sla, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x23, "SLA E", op_sla, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x24, "SLA H", op_sla, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x25, "SLA L", op_sla, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x26, "SLA (HL)", op_sla, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x27, "SLA A", op_sla, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x28, "SRA B", op_sra, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x29, "SRA C", op_sra, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x2A, "SRA D", op_sra, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x2B, "SRA E", op_sra, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x2C, "SRA H", op_sra, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x2D, "SRA L", op_sra, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x2E, "SRA (HL)", op_sra, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x2F, "SRA A", op_sra, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x30, "SWAP B", op_swap, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x31, "SWAP C", op_swap, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x32, "SWAP D", op_swap, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x33, "SWAP E", op_swap, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x34, "SWAP H", op_swap, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x35, "SWAP L", op_swap, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x36, "SWAP (HL)", op_swap, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x37, "SWAP A", op_swap, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x38, "SRL B", op_srl, B, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x39, "SRL C", op_srl, C, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x3A, "SRL D", op_srl, D, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x3B, "SRL E", op_srl, E, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x3C, "SRL H", op_srl, H, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x3D, "SRL L", op_srl, L, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x3E, "SRL (HL)", op_srl, HL, NULL, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x3F, "SRL A", op_srl, A, NULL, NONE, 2, 8, 0);
SET_OP_CB(0x40, "BIT 0,B", op_bit, B, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x41, "BIT 0,C", op_bit, C, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x42, "BIT 0,D", op_bit, D, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x43, "BIT 0,E", op_bit, E, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x44, "BIT 0,H", op_bit, H, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x45, "BIT 0,L", op_bit, L, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x46, "BIT 0,(HL)", op_bit, HL, &C_0, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x47, "BIT 0,A", op_bit, A, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x48, "BIT 1,B", op_bit, B, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x49, "BIT 1,C", op_bit, C, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x4A, "BIT 1,D", op_bit, D, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x4B, "BIT 1,E", op_bit, E, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x4C, "BIT 1,H", op_bit, H, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x4D, "BIT 1,L", op_bit, L, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x4E, "BIT 1,(HL)", op_bit, HL, &C_1, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x4F, "BIT 1,A", op_bit, A, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x50, "BIT 2,B", op_bit, B, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x51, "BIT 2,C", op_bit, C, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x52, "BIT 2,D", op_bit, D, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x53, "BIT 2,E", op_bit, E, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x54, "BIT 2,H", op_bit, H, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x55, "BIT 2,L", op_bit, L, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x56, "BIT 2,(HL)", op_bit, HL, &C_2, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x57, "BIT 2,A", op_bit, A, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x58, "BIT 3,B", op_bit, B, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x59, "BIT 3,C", op_bit, C, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x5A, "BIT 3,D", op_bit, D, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x5B, "BIT 3,E", op_bit, E, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x5C, "BIT 3,H", op_bit, H, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x5D, "BIT 3,L", op_bit, L, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x5E, "BIT 3,(HL)", op_bit, HL, &C_3, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x5F, "BIT 3,A", op_bit, A, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x60, "BIT 4,B", op_bit, B, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0x61, "BIT 4,C", op_bit, C, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0x62, "BIT 4,D", op_bit, D, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0x63, "BIT 4,E", op_bit, E, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0x64, "BIT 4,H", op_bit, H, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0x65, "BIT 4,L", op_bit, L, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0x66, "BIT 4,(HL)", op_bit, HL, &C_4, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x67, "BIT 4,A", op_bit, A, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0x68, "BIT 5,B", op_bit, B, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0x69, "BIT 5,C", op_bit, C, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0x6A, "BIT 5,D", op_bit, D, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0x6B, "BIT 5,E", op_bit, E, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0x6C, "BIT 5,H", op_bit, H, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0x6D, "BIT 5,L", op_bit, L, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0x6E, "BIT 5,(HL)", op_bit, HL, &C_5, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x6F, "BIT 5,A", op_bit, A, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0x70, "BIT 6,B", op_bit, B, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0x71, "BIT 6,C", op_bit, C, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0x72, "BIT 6,D", op_bit, D, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0x73, "BIT 6,E", op_bit, E, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0x74, "BIT 6,H", op_bit, H, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0x75, "BIT 6,L", op_bit, L, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0x76, "BIT 6,(HL)", op_bit, HL, &C_6, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x77, "BIT 6,A", op_bit, A, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0x78, "BIT 7,B", op_bit, B, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0x79, "BIT 7,C", op_bit, C, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0x7A, "BIT 7,D", op_bit, D, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0x7B, "BIT 7,E", op_bit, E, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0x7C, "BIT 7,H", op_bit, H, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0x7D, "BIT 7,L", op_bit, L, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0x7E, "BIT 7,(HL)", op_bit, HL, &C_7, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x7F, "BIT 7,A", op_bit, A, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0x80, "RES 0,B", op_res, B, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x81, "RES 0,C", op_res, C, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x82, "RES 0,D", op_res, D, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x83, "RES 0,E", op_res, E, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x84, "RES 0,H", op_res, H, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x85, "RES 0,L", op_res, L, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x86, "RES 0,(HL)", op_res, HL, &C_0, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x87, "RES 0,A", op_res, A, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0x88, "RES 1,B", op_res, B, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x89, "RES 1,C", op_res, C, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x8A, "RES 1,D", op_res, D, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x8B, "RES 1,E", op_res, E, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x8C, "RES 1,H", op_res, H, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x8D, "RES 1,L", op_res, L, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x8E, "RES 1,(HL)", op_res, HL, &C_1, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x8F, "RES 1,A", op_res, A, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0x90, "RES 2,B", op_res, B, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x91, "RES 2,C", op_res, C, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x92, "RES 2,D", op_res, D, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x93, "RES 2,E", op_res, E, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x94, "RES 2,H", op_res, H, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x95, "RES 2,L", op_res, L, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x96, "RES 2,(HL)", op_res, HL, &C_2, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x97, "RES 2,A", op_res, A, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0x98, "RES 3,B", op_res, B, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x99, "RES 3,C", op_res, C, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x9A, "RES 3,D", op_res, D, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x9B, "RES 3,E", op_res, E, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x9C, "RES 3,H", op_res, H, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x9D, "RES 3,L", op_res, L, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0x9E, "RES 3,(HL)", op_res, HL, &C_3, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0x9F, "RES 3,A", op_res, A, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xA0, "RES 4,B", op_res, B, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xA1, "RES 4,C", op_res, C, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xA2, "RES 4,D", op_res, D, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xA3, "RES 4,E", op_res, E, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xA4, "RES 4,H", op_res, H, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xA5, "RES 4,L", op_res, L, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xA6, "RES 4,(HL)", op_res, HL, &C_4, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xA7, "RES 4,A", op_res, A, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xA8, "RES 5,B", op_res, B, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xA9, "RES 5,C", op_res, C, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xAA, "RES 5,D", op_res, D, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xAB, "RES 5,E", op_res, E, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xAC, "RES 5,H", op_res, H, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xAD, "RES 5,L", op_res, L, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xAE, "RES 5,(HL)", op_res, HL, &C_5, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xAF, "RES 5,A", op_res, A, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xB0, "RES 6,B", op_res, B, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xB1, "RES 6,C", op_res, C, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xB2, "RES 6,D", op_res, D, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xB3, "RES 6,E", op_res, E, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xB4, "RES 6,H", op_res, H, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xB5, "RES 6,L", op_res, L, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xB6, "RES 6,(HL)", op_res, HL, &C_6, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xB7, "RES 6,A", op_res, A, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xB8, "RES 7,B", op_res, B, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xB9, "RES 7,C", op_res, C, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xBA, "RES 7,D", op_res, D, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xBB, "RES 7,E", op_res, E, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xBC, "RES 7,H", op_res, H, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xBD, "RES 7,L", op_res, L, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xBE, "RES 7,(HL)", op_res, HL, &C_7, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xBF, "RES 7,A", op_res, A, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xC0, "SET 0,B", op_set, B, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0xC1, "SET 0,C", op_set, C, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0xC2, "SET 0,D", op_set, D, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0xC3, "SET 0,E", op_set, E, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0xC4, "SET 0,H", op_set, H, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0xC5, "SET 0,L", op_set, L, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0xC6, "SET 0,(HL)", op_set, HL, &C_0, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xC7, "SET 0,A", op_set, A, &C_0, NONE, 2, 8, 0);
SET_OP_CB(0xC8, "SET 1,B", op_set, B, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0xC9, "SET 1,C", op_set, C, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0xCA, "SET 1,D", op_set, D, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0xCB, "SET 1,E", op_set, E, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0xCC, "SET 1,H", op_set, H, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0xCD, "SET 1,L", op_set, L, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0xCE, "SET 1,(HL)", op_set, HL, &C_1, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xCF, "SET 1,A", op_set, A, &C_1, NONE, 2, 8, 0);
SET_OP_CB(0xD0, "SET 2,B", op_set, B, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0xD1, "SET 2,C", op_set, C, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0xD2, "SET 2,D", op_set, D, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0xD3, "SET 2,E", op_set, E, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0xD4, "SET 2,H", op_set, H, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0xD5, "SET 2,L", op_set, L, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0xD6, "SET 2,(HL)", op_set, HL, &C_2, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xD7, "SET 2,A", op_set, A, &C_2, NONE, 2, 8, 0);
SET_OP_CB(0xD8, "SET 3,B", op_set, B, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xD9, "SET 3,C", op_set, C, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xDA, "SET 3,D", op_set, D, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xDB, "SET 3,E", op_set, E, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xDC, "SET 3,H", op_set, H, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xDD, "SET 3,L", op_set, L, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xDE, "SET 3,(HL)", op_set, HL, &C_3, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xDF, "SET 3,A", op_set, A, &C_3, NONE, 2, 8, 0);
SET_OP_CB(0xE0, "SET 4,B", op_set, B, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xE1, "SET 4,C", op_set, C, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xE2, "SET 4,D", op_set, D, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xE3, "SET 4,E", op_set, E, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xE4, "SET 4,H", op_set, H, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xE5, "SET 4,L", op_set, L, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xE6, "SET 4,(HL)", op_set, HL, &C_4, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xE7, "SET 4,A", op_set, A, &C_4, NONE, 2, 8, 0);
SET_OP_CB(0xE8, "SET 5,B", op_set, B, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xE9, "SET 5,C", op_set, C, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xEA, "SET 5,D", op_set, D, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xEB, "SET 5,E", op_set, E, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xEC, "SET 5,H", op_set, H, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xED, "SET 5,L", op_set, L, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xEE, "SET 5,(HL)", op_set, HL, &C_5, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xEF, "SET 5,A", op_set, A, &C_5, NONE, 2, 8, 0);
SET_OP_CB(0xF0, "SET 6,B", op_set, B, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xF1, "SET 6,C", op_set, C, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xF2, "SET 6,D", op_set, D, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xF3, "SET 6,E", op_set, E, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xF4, "SET 6,H", op_set, H, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xF5, "SET 6,L", op_set, L, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xF6, "SET 6,(HL)", op_set, HL, &C_6, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xF7, "SET 6,A", op_set, A, &C_6, NONE, 2, 8, 0);
SET_OP_CB(0xF8, "SET 7,B", op_set, B, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xF9, "SET 7,C", op_set, C, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xFA, "SET 7,D", op_set, D, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xFB, "SET 7,E", op_set, E, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xFC, "SET 7,H", op_set, H, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xFD, "SET 7,L", op_set, L, &C_7, NONE, 2, 8, 0);
SET_OP_CB(0xFE, "SET 7,(HL)", op_set, HL, &C_7, MEM_RW_16, 2, 16, 0);
SET_OP_CB(0xFF, "SET 7,A", op_set, A, &C_7, NONE, 2, 8, 0);
}

void cpu_init() {
	set_opcodes();
	set_opcodes_cb();
	PC = 0;
	*F = 0x00;
}
