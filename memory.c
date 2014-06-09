static uint8_t mm[0xFFFF];

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
