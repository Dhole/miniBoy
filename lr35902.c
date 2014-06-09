#include <stdio.h>
#include <stdint.h>

uint16_t SP, PC;
uint16_t AF, BC, DE, HL;
uint8_t *F = (uint8_t*)&AF, *A = (uint8_t*)&AF + 1,
	*C = (uint8_t*)&BC, *B = (uint8_t*)&BC + 1,
	*E = (uint8_t*)&DE, *D = (uint8_t*)&DE + 1,
	*L = (uint8_t*)&HL, *H = (uint8_t*)&HL + 1;

uint8_t mm[0xFFFF];

uint8_t read_8(uint16_t addr) {
	return mm[addr];
}

uint16_t read_16(uint16_t addr) {
	return mm[addr] + mm[addr + 1] << 8;
}

void write_8(uint16_t addr, uint8_t v) {
	mm[addr] = v;
}

void write_16(uint16_t addr, uint16_t v) {
	mm[addr] = v && 0x00FF;
	mm[addr + 1] = v && 0xFF00;
}

int main() {
	AF = 256;
	printf("AF: %i\n", AF);
	printf("A: %i, F: %i\n", *A, *F);
	write_8(1, 1);
	printf("0: %i, 1: %i, 01: %i\n", read_8(0), read_8(1), read_16(0));
	return 0;
}

