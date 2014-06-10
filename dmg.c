#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lr35902.h"
#include "memory.h"

#define MAX_BIOS_SIZE 0x100
#define MAX_ROM_SIZE 0x400000

static uint8_t bios[256];
static uint8_t *rom;

void load_bios(char *bios_path) {
	int size;
	FILE *fb;
	fb = fopen(bios_path, "rb");
	if (fb == NULL) {
		printf("Can't open bios file %s", bios_path);
		exit(1);
	}
	fseek(fb, 0, SEEK_END);
	size = ftell(fb);
	if (size > MAX_BIOS_SIZE) {
		printf("Bios file %s too big", bios_path);
		exit(1);
	}
	fseek(fb, 0, SEEK_SET);
	fread(bios, 1, size, fb);
	mem_load(0, bios, size);
	fclose(fb);
}

void load_rom(char *rom_path) {
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
	fseek(fr, 0, SEEK_SET);
	rom = malloc(size);
	fread(rom, 1, size, fr);
	mem_load(0x100, &rom[0x100], 0x4000 - 0x100);
	fclose(fr);
}

void unload_rom() {
	free(rom);
}

void init() {
	cpu_init();
}
