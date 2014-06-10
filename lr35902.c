#include <stdio.h>
#include <stdint.h>

static uint16_t SP, PC;
static uint16_t AF, BC, DE, HL;
static uint8_t *F = (uint8_t*)&AF, *A = (uint8_t*)&AF + 1,
	*C = (uint8_t*)&BC, *B = (uint8_t*)&BC + 1,
	*E = (uint8_t*)&DE, *D = (uint8_t*)&DE + 1,
	*L = (uint8_t*)&HL, *H = (uint8_t*)&HL + 1;

/*
int main() {
	AF = 256;
	printf("AF: %i\n", AF);
	printf("A: %i, F: %i\n", *A, *F);
	write_8(1, 1);
	printf("0: %i, 1: %i, 01: %i\n", read_8(0), read_8(1), read_16(0));
	return 0;
}
*/
