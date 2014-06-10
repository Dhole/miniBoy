#include <stdint.h>
#include <string.h>

static uint8_t mm[0xFFFF];

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
