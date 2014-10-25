#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lr35902.h"
#include "memory.h"

static regs_t *regs;
static uint8_t *mm;

#define ass_flagsZNHC(z, n, h, c)		\
	assert(get_flag(Z_FLAG) == z);		\
	assert(get_flag(N_FLAG) == n);		\
	assert(get_flag(H_FLAG) == h);		\
	assert(get_flag(C_FLAG) == c);

#define ass_PC(v) assert(regs->PC == v);
#define ass_SP(v) assert(*(regs->SP) == v);
#define ass_AF(v) assert(*(regs->AF) == v);
#define ass_BC(v) assert(*(regs->BC) == v);
#define ass_DE(v) assert(*(regs->DE) == v);
#define ass_HL(v) assert(*(regs->HL) == v);
#define ass_F(v) assert(*(regs->F) == v);
#define ass_A(v) assert(*(regs->A) == v);
#define ass_C(v) assert(*(regs->C) == v);
#define ass_B(v) assert(*(regs->B) == v);
#define ass_E(v) assert(*(regs->E) == v);
#define ass_D(v) assert(*(regs->D) == v);
#define ass_L(v) assert(*(regs->L) == v);
#define ass_H(v) assert(*(regs->H) == v);

#define ass_mm(a, v) assert(mm[a] == v);

#define set_PC(v) regs->PC = v;
#define set_SP(v) *(regs->SP) = v;
#define set_AF(v) *(regs->AF) = v;
#define set_BC(v) *(regs->BC) = v;
#define set_DE(v) *(regs->DE) = v;
#define set_HL(v) *(regs->HL) = v;
#define set_F(v) *(regs->F) = v;
#define set_A(v) *(regs->A) = v;
#define set_C(v) *(regs->C) = v;
#define set_B(v) *(regs->B) = v;
#define set_E(v) *(regs->E) = v;
#define set_D(v) *(regs->D) = v;
#define set_L(v) *(regs->L) = v;
#define set_H(v) *(regs->H) = v;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"

void test_init() {
	cpu_init();
	regs = cpu_get_regs();
	mm = mem_get_mem();
}

void test_cpu_reset() {
	regs->PC = 0;
	*(regs->SP) = 0;
	*(regs->AF) = 0;
	*(regs->BC) = 0;
	*(regs->DE) = 0;
	*(regs->HL) = 0;
}

void test_mem_reset() {
	int i;
	for (i = 0; i < MEM_SIZE; i++) {
		mm[i] = 0;
	}
}

void test_reset() {
	test_cpu_reset();
	test_mem_reset();
}

void test_op_nop() {
	mem_write_8(0, 0x00);
	cpu_step();
	ass_PC(1);
	ass_flagsZNHC(0, 0, 0, 0);
}

void test_op_inc_16() {
	mem_write_8(0, 0x03);
	mem_write_8(1, 0x13);

	set_DE(0xffff);

	cpu_step();
	ass_BC(0x0001);
	ass_flagsZNHC(0, 0, 0, 0);

	cpu_step();	
	ass_DE(0x0000);
	ass_flagsZNHC(0, 0, 0, 0);

	ass_PC(2);
}

void test_op_dec_16() {
	mem_write_8(0, 0x0B);
	mem_write_8(1, 0x1B);

	set_DE(0xffff);

	cpu_step();
	ass_BC(0xffff);
	ass_flagsZNHC(0, 0, 0, 0);

	cpu_step();	
	ass_DE(0xfffe);
	ass_flagsZNHC(0, 0, 0, 0);

	ass_PC(2);
}

void test_op_inc_8() {
	mem_write_8(0, 0x04);
	mem_write_8(1, 0x14);
	mem_write_8(2, 0x0C);
	
	mem_write_8(3, 0x34);
	mem_write_8(4, 0x34);
	mem_write_8(5, 0x34);

	set_B(0x0);
	set_D(0xf);
	set_C(0xff);

	mem_write_8(0x10, 0);
	mem_write_8(0x11, 0xf);
	mem_write_8(0x12, 0xff);

	cpu_step();
	ass_B(0x1);
	ass_flagsZNHC(0, 0, 0, 0);

	cpu_step();
	ass_D(0x10);
	ass_flagsZNHC(0, 0, 1, 0);

	cpu_step();
	ass_C(0x0);
	ass_flagsZNHC(1, 0, 1, 0);

	
	set_HL(0x10);
	cpu_step();
	ass_mm(0x10, 0x1);
	ass_flagsZNHC(0, 0, 0, 0);

	set_HL(0x11);
	cpu_step();
	ass_mm(0x11, 0x10);
	ass_flagsZNHC(0, 0, 1, 0);

	set_HL(0x12);
	cpu_step();
	ass_mm(0x12, 0x0);
	ass_flagsZNHC(1, 0, 1, 0);	
}

void test_op_dec_8() {
	mem_write_8(0, 0x05);
	mem_write_8(1, 0x15);
	mem_write_8(2, 0x0D);

	mem_write_8(3, 0x35);
	mem_write_8(4, 0x35);
	mem_write_8(5, 0x35);

	set_B(0x5);
	set_D(0x10);
	set_C(0x0);

	mem_write_8(0x10, 0x5);
	mem_write_8(0x11, 0x10);
	mem_write_8(0x12, 0x1);

	cpu_step();
	ass_B(0x4);
	ass_flagsZNHC(0, 1, 0, 0);

	cpu_step();
	//printf("%2x \n", *(regs->D));
	ass_D(0xf);
	ass_flagsZNHC(0, 1, 1, 0);

	cpu_step();
	ass_C(0xff);
	ass_flagsZNHC(0, 1, 1, 0);
}

void test_op_ld_8() {
	mem_write_8(0, 0x7E);
	mem_write_8(1, 0x70);
	mem_write_8(2, 0x60);

	mem_write_8(0x10, 0x33);
	set_HL(0x10);
	set_A(0x55);
	set_B(0x66);
	
	cpu_step();
	ass_A(0x33);

	cpu_step();
	ass_mm(0x10, 0x66);

	cpu_step();
	ass_H(0x66);
}

void test_op_ldi() {
	mem_write_8(0, 0x22);
	mem_write_8(1, 0x2A);
	mem_write_8(2, 0x32);
	mem_write_8(3, 0x3A);

	mem_write_8(0x10, 0x55);
	mem_write_8(0x11, 0x66);
	set_HL(0x10);
	set_A(0x33);

	cpu_step();
	ass_mm(0x10, 0x33);
	ass_HL(0x11);
	ass_PC(1);

	cpu_step();
	ass_A(0x66);
	ass_HL(0x12);
	ass_PC(2);

	cpu_step();
	ass_mm(0x12, 0x66);
	ass_HL(0x11);
	ass_PC(3);

	cpu_step();
	ass_A(0x66);
	ass_HL(0x10);
	ass_PC(4);
}

void test_op_add_8() {
	mem_write_8(0, 0x80);
	mem_write_8(1, 0x81);
	mem_write_8(2, 0x82);
	mem_write_8(3, 0x86);
	mem_write_8(4, 0xC6);
	mem_write_8(5, 0x13);
	
	set_HL(0x06);
	mem_write_8(6, 0xF1);

	set_A(0x0D);
	set_B(0x02);
	set_C(0x01);
	set_D(0xFF);	

	cpu_step();
	ass_A(0x0F);
	ass_flagsZNHC(0, 0, 0, 0);

	cpu_step();
	ass_A(0x10);
	ass_flagsZNHC(0, 0, 1, 0);

	cpu_step();
	ass_A(0x0F);
	ass_flagsZNHC(0, 0, 0, 1);

	cpu_step();
	ass_A(0x00);
	ass_flagsZNHC(1, 0, 1, 1);

	cpu_step();
	ass_A(0x13);
	ass_flagsZNHC(0, 0, 0, 0);
}

void test_op_sub_8() {
	mem_write_8(0, 0x90);
	mem_write_8(1, 0x91);
	mem_write_8(2, 0x92);
	mem_write_8(3, 0x96);
	mem_write_8(4, 0xD6);
	mem_write_8(5, 0xE0);

	set_HL(0x06);
	mem_write_8(6, 0x10);

	set_A(0x12);
	set_B(0x02);
	set_C(0x01);
	set_D(0x0F);

	cpu_step();
	ass_A(0x10);
	ass_flagsZNHC(0, 1, 0, 0);

	cpu_step();
	ass_A(0x0F);
	ass_flagsZNHC(0, 1, 1, 0);

	cpu_step();
	ass_A(0x00);
	ass_flagsZNHC(1, 1, 0, 0);

	cpu_step();
	ass_A(0xF0);
	ass_flagsZNHC(0, 1, 0, 1);

	cpu_step();
	ass_A(0x10);
	ass_flagsZNHC(0, 1, 0, 0);
}

void test_op_and() {
	mem_write_8(0, 0xA0);
	mem_write_8(1, 0xA1);
	mem_write_8(2, 0xA6);
	mem_write_8(3, 0xE6);
	mem_write_8(4, 0x11);

	set_HL(0x05);
	mem_write_8(5, 0x10);

	set_A(0xFE);
	set_B(0x0F);
	set_C(0x03);

	cpu_step();
	ass_A(0x0E);
	ass_flagsZNHC(0, 0, 1, 0);

	cpu_step();
	ass_A(0x02);
	ass_flagsZNHC(0, 0, 1, 0);

	cpu_step();
	ass_A(0x00);
	ass_flagsZNHC(1, 0, 1, 0);

	set_A(0xF0);
	
	cpu_step();
	ass_A(0x10);
	ass_flagsZNHC(0, 0, 1, 0);
}

void test_op_or() {
	mem_write_8(0, 0xB0);
	mem_write_8(1, 0xB1);
	mem_write_8(2, 0xB6);
	mem_write_8(3, 0xF6);
	mem_write_8(4, 0xC0);

	set_HL(0x05);
	mem_write_8(5, 0x2E);

	set_A(0x00);
	set_B(0x00);
	set_C(0x11);

	cpu_step();
	ass_A(0x00);
	ass_flagsZNHC(1, 0, 0, 0);

	cpu_step();
	ass_A(0x11);
	ass_flagsZNHC(0, 0, 0, 0);

	cpu_step();
	ass_A(0x3F);
	ass_flagsZNHC(0, 0, 0, 0);

	cpu_step();
	ass_A(0xFF);
	ass_flagsZNHC(0, 0, 0, 0);
}

void test_op_xor() {
	mem_write_8(0, 0xEE);
	mem_write_8(1, 0x00);

	set_A(0xCC);

	cpu_step();
	ass_A(0xCC);
	ass_flagsZNHC(0, 0, 0, 0);
}

int main() {
	test_init();

	test_reset(); test_op_inc_8();
	test_reset(); test_op_inc_16();
	test_reset(); test_op_dec_16();
	test_reset(); test_op_nop();
	test_reset(); test_op_ld_8();
	test_reset(); test_op_ldi();
	test_reset(); test_op_dec_8();
	// ld_16
	// ldd
	// ldhl
	test_reset(); test_op_add_8();
	// add_16
	// addsp
	// adc
	test_reset(); test_op_sub_8();
	// sbc
	test_reset(); test_op_and();
	test_reset(); test_op_xor();
	test_reset(); test_op_or();
	// cp
	// ccf
	// scf
	// rlca
	// rla
	// rrca
	// rra
	// push
	// pop
	// jp
	// jr
	// call
	// ret
	// rst
	// cpl
	// daa
	// CB ...
	// rlc
	// rrc
	// rl
	// rr
	// sla
	// sra
	// swap
	// srl
	// bit
	// res
	// set

	printf("All tests passed\n");
	
	return 0;
}

#pragma clang diagnostic pop
