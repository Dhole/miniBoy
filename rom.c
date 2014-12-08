#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "rom.h"

#define MBC_RAM_MODE 0
#define MBC_ROM_MODE 1

static rom_info_t rom_info;
static uint8_t *rom;
static uint8_t *ram;

static uint8_t rom_bank;
static uint8_t ram_bank;
static uint8_t ram_enable;
static uint8_t rom_ram_mode;

void rom_init() {
	rom = NULL;
	rom = NULL;
	rom_info.title[0] = 0x00;
	rom_bank = 0x01;
	ram_bank = 0x00;
	rom_ram_mode = MBC_ROM_MODE;
	// set the rest...
}

void rom_load(char *rom_path) {
	int size;
	FILE *fr;
	
	fr = fopen(rom_path, "rb");
	if (fr == NULL) {
		printf("Can't open rom file %s", rom_path);
		exit(1);
	}
	fseek(fr, 0, SEEK_END);
	size = ftell(fr);
	if (size > MAX_ROM_SIZE) {
		printf("Rom file %s too big", rom_path);
		exit(1);
	}
	if (size < MIN_ROM_SIZE) {
		printf("Rom file %s too small", rom_path);
		exit(1);
	}
	fseek(fr, 0, SEEK_SET);
	rom = malloc(size);
	fread(rom, 1, size, fr);
	//mem_set_rom(rom, size);
	//mem_load_rom();
	fclose(fr);
	if (rom_read_info()) {
		exit(1);
	}
	ram = malloc(rom_info.ram_size);
}

void rom_unload() {
	if (rom != NULL) {
		free(rom);
		rom = NULL;
	} else {
		printf("Error, no rom loaded to unload!\n");
		exit(1);
	}
	if (ram != NULL) {
		free(ram);
		ram = NULL;
	}
}

int rom_read_info() {
	memcpy(rom_info.title, &rom[ROM_POS_TITLE], ROM_TITLE_LEN);
	rom_info.title[ROM_TITLE_LEN] = 0x00;

	switch(rom[ROM_POS_ROM_SIZE]) {
	case ROM_256Kbit:
		rom_info.rom_size = (256 * 1024) / 8;
		break;
	case ROM_512Kbit:
		rom_info.rom_size = (512 * 1024) / 8;
		break;
	case ROM_1Mbit:
		rom_info.rom_size = (1 * 1024 * 1024) / 8;
		break;
	case ROM_2Mbit:
		rom_info.rom_size =  (2 * 1024 * 1024)/ 8;
		break;
	case ROM_4Mbit:
		rom_info.rom_size =  (4 * 1024 * 1024)/ 8;
		break;
	case ROM_8Mbit:
		rom_info.rom_size =  (8 * 1024 * 1024)/ 8;
		break;
	case ROM_16Mbit:
		rom_info.rom_size =  (16 * 1024 * 1024)/ 8;
		break;
	case ROM_9Mbit:
		rom_info.rom_size =  (9 * 1024 * 1024)/ 8;
		break;
	case ROM_10Mbit:
		rom_info.rom_size =  (10 * 1024 * 1024)/ 8;
		break;
	case ROM_12Mbit:
		rom_info.rom_size =  (12 * 1024 * 1024)/ 8;
		break;
	default:
		printf("ROM size not recognized\n");
		return 1;
	}

	switch(rom[ROM_POS_RAM_SIZE]) {
	case RAM_NONE:
		rom_info.ram_size = 0;
		break;
	case RAM_16Kbit:
		rom_info.ram_size = (16 * 1024) / 8;
		break;
	case RAM_64Kbit:
		rom_info.ram_size = (64 * 1024) / 8;
		break;
	case RAM_256Kbit:
		rom_info.ram_size = (256 * 1024) / 8;
		break;
	case RAM_1Mbit:
		rom_info.ram_size = (1 * 1024 * 1024) / 8;
		break;
	default:
		printf("RAM size not recognized\n");
		return 1;
	}

	switch(rom[ROM_POS_CART_TYPE]) {
	case ROM_ONLY:
		rom_info.mbc = MBC_NONE;
		break;
	case ROM_MBC1:
		rom_info.mbc = MBC_1;
		break;
	case ROM_MBC1_RAM:
		rom_info.mbc = MBC_1;
		break;
	case ROM_MBC1_RAM_BATT:
		rom_info.mbc = MBC_1;
		break;
	case ROM_MBC2:
		rom_info.mbc = MBC_2;
		break;
	case ROM_MBC2_BATT:
		rom_info.mbc = MBC_2;
		break;
	case ROM_RAM:
		break;
	case ROM_RAM_BATT:
		break;
	case ROM_MMM01:
		break;
	case ROM_MMM01_SRAM:
		break;
	case ROM_MMM01_SRAM_BATT:
		break;
	case ROM_MBC3_TIMER_BATT:
		rom_info.mbc = MBC_3;
		break;
	case ROM_MBC3_TIMER_RAM_BATT:
		rom_info.mbc = MBC_3;
		break;
	case ROM_MBC3:
		rom_info.mbc = MBC_3;
		break;
	case ROM_MBC3_RAM:
		rom_info.mbc = MBC_3;
		break;
	case ROM_MBC3_RAM_BATT:
		rom_info.mbc = MBC_3;
		break;
	case ROM_MBC5:
		rom_info.mbc = MBC_5;
		break;
	case ROM_MBC5_RAM:
		rom_info.mbc = MBC_5;
		break;
	case ROM_MBC5_RAM_BATT:
		rom_info.mbc = MBC_5;
		break;
	case ROM_MBC5_RUMBLE:
		rom_info.mbc = MBC_5;
		break;
	case ROM_MBC5_RUMB_E_SRAM:
		rom_info.mbc = MBC_5;
		break;
	case ROM_MBC5_RUMBLE_SRAM_BATT:
		rom_info.mbc = MBC_5;
		break;
	case ROM_PocketCamera:
		break;
	case ROM_BandaiTAMA5:
		break;
	case ROM_HudsonHuC3:
		break;
	case ROM_HudsonHuC1:
		break;
	default:
		printf("Cartridge type not recocniged\n");
		return 1;
	}


	return 0;
}

void rom_print_info() {

}

uint8_t mbc_none_read_8(uint16_t addr) {
	if (addr < 0x8000) {
		return rom[addr];
	}
	return 0;	
}

void mbc_none_write_8(uint16_t addr, uint8_t v) {
	return;
}

uint8_t mbc1_read_8(uint16_t addr) {
	if (addr < 0x4000) {
		return rom[addr];
	}
	if (addr < 0x8000) {
		return rom[addr + 0x4000 * (rom_bank - 1)];
	}
	if (addr < 0xC000) {
		return ram[addr - 0xA000 + 0x2000 * ram_bank];
	}
	return 0;
}

void mbc1_write_8(uint16_t addr, uint8_t v) {
	if (addr < 0x2000) {
		ram_enable = 1;
		return;
	}
	if (addr < 0x4000) {
		v &= 0x1F;
		rom_bank = (rom_bank & 0xE0) | v;
		if (v == 0x00) {
			rom_bank |= 0x01;
		}
		return;
	}
	if (addr < 0x6000) {
		if (rom_ram_mode == MBC_ROM_MODE) {
			v &= 0x07;
			rom_bank = (rom_bank & 0x1F) | (v << 5);
		} else {
			ram_bank = v & 0x03;
		}
		return;
	}
}

uint8_t rom_read_8(uint16_t addr) {
	switch (rom_info.mbc) {
	case MBC_NONE:
		return mbc_none_read_8(addr);
		break;
	case MBC_1:
		return mbc1_read_8(addr);
		break;
	case MBC_2:
		break;
	case MBC_3:
		break;
	case MBC_5:
		break;
	}
	return 0;
}
void rom_write_8(uint16_t addr, uint8_t v){
	switch (rom_info.mbc) {
	case MBC_NONE:
		mbc_none_write_8(addr, v);
		break;
	case MBC_1:
		mbc1_write_8(addr, v);
		break;
	case MBC_2:
		break;
	case MBC_3:
		break;
	case MBC_5:
		break;
	}
}

