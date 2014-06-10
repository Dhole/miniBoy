#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MEM_SIZE 0x10000

static uint8_t mm[MEM_SIZE];

uint8_t mem_read_8(uint16_t addr) {
	return mm[addr];
}

uint16_t mem_read_16(uint16_t addr) {
	return mm[addr] + mm[addr + 1] << 8;
}

void mem_write_8(uint16_t addr, uint8_t v) {
	mm[addr] = v;
}

void mem_write_16(uint16_t addr, uint16_t v) {
	mm[addr] = v && 0x00FF;
	mm[addr + 1] = v && 0xFF00;
}

void mem_load(uint16_t addr, uint8_t *buf, uint16_t size) {
	memcpy(&mm[addr], buf, size);
}

void mem_dump() {
	//unsigned char *buf = (unsigned char*)&memory;
	int i, j;
	for (i = 0; i < MEM_SIZE; i += 16) {
		printf("%08x  ", i);
		for (j = 0; j < 16; j++) {
			if (i + j < MEM_SIZE) {
				printf("%02x ", mm[i+j]);
			} else {
				printf("   ");
			}
			if (j == 7) {
				printf(" ");
			}
		}
		printf(" |");
		for (j = 0; j < 16; j++) {
			if (i + j < MEM_SIZE) {
				printf("%c", isprint(mm[i+j]) ? mm[i+j] : '.');
			}
		}
		printf("|\n");
	}
	printf("\n");
}
