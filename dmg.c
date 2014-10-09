#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lr35902.h"
#include "memory.h"
#include "screen.h"
#include "io_regs.h"
#include "debugger.h"

#define MAX_BIOS_SIZE 0x100
#define MAX_ROM_SIZE 0x400000

//static uint8_t bios[256];
//static uint8_t *rom;

void dmg_load_bios(char *bios_path) {
	int size;
	FILE *fb;
	uint8_t *bios;
	
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
	bios = malloc(size);
	fread(bios, 1, size, fb);
	//mem_load(0, bios, size);
	mem_set_bios(bios, size);
	fclose(fb);
}

void dmg_load_rom(char *rom_path) {
	int size;
	FILE *fr;
	uint8_t *rom;
	
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
	//mem_load(0x100, &rom[0x100], 0x4000 - 0x100);
	//printf("READ ROM: %02x %02x %02x %02x\n", rom[0], rom[1], rom[2], rom[3]);
	mem_set_rom(rom, size);
	mem_load_rom();
	fclose(fr);
}

void dmg_unload_rom() {
	mem_unset_rom();
}

void dmg_init() {
	mem_enable_bios();
	debug_init();
	cpu_init();
	//cpu_test();
	//debug_run();
}

void dmg_reset() {
	
}

void dmg_run(uint32_t delta, int *debug_flag, int *debug_pause) {
	int clk, cycles;
	//dmg_emulate_hardware(delta);
	clk = 0;
	screen_start_frame();
		
	while (clk < SCREEN_DUR_FRAME) {
		// if timer overflow...
		// if End of serial IO transfer...
		// if High to low p10-p13...

		if (*debug_flag) {
			cycles = debug_run(debug_flag, debug_pause);
		} else {
			cycles = cpu_step();	
		}
			
		// If LCD on...!!!
		screen_emulate(cycles);
			
		clk += cycles;
	}
	//screen_write_fb();
}
