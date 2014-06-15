#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MEM_SIZE 0x10000

static uint8_t mm[MEM_SIZE];

uint8_t mem_read_8(uint16_t addr) {
	return mm[addr];
}

uint16_t mem_read_16(uint16_t addr) {
	return ((uint16_t)mm[addr] << 8) + (uint16_t)mm[addr + 1];
}

void mem_write_8(uint16_t addr, uint8_t v) {
	mm[addr] = v;
}

void mem_write_16(uint16_t addr, uint16_t v) {
	mm[addr] = (uint8_t)(v && 0x00FF);
	mm[addr + 1] = (uint8_t)((v && 0xFF00) >> 8);
}

void mem_load(uint16_t addr, uint8_t *buf, uint16_t size) {
	memcpy(&mm[addr], buf, size);
}

void mem_dump(uint16_t start, uint16_t end) {
	//unsigned char *buf = (unsigned char*)&memory;
	int i, j;
	for (i = start; i < end; i += 16) {
		printf("%04x  ", i);
		for (j = 0; j < 16; j++) {
			if (i + j < end) {
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
			if (i + j < end) {
				printf("%c", isprint(mm[i+j]) ? mm[i+j] : '.');
			}
		}
		printf("|\n");
	}
}
