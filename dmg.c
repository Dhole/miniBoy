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
static uint8_t skip_bios = 0;

void dmg_load_bios(char *bios_path) {
	int size;
	FILE *fb;
	uint8_t *bios;

	if (bios_path == NULL) {
		printf("Booting without BIOS ROM\n");
		skip_bios = 1;
		return;
	}
	
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

void set_boot_state() {
	regs_t *regs;

	regs = cpu_get_regs();
	(*regs).PC = 0x100;
	*(regs->AF) = 0x01B0;
	*(regs->BC) = 0x0013;
	*(regs->DE) = 0x00D8;
	*(regs->HL) = 0x014D;
	
	*(regs->SP) = 0xFFFE;

	mem_write_8(0xFF05, 0x00);
	mem_write_8(0xFF06, 0x00);
	mem_write_8(0xFF07, 0x00);
	mem_write_8(0xFF10, 0x80);
	mem_write_8(0xFF11, 0xBF);
	mem_write_8(0xFF12, 0xF3);
	mem_write_8(0xFF14, 0xBF);
	mem_write_8(0xFF16, 0x3F);
	mem_write_8(0xFF17, 0x00);
	mem_write_8(0xFF19, 0xBF);
	mem_write_8(0xFF1A, 0x7F);
	mem_write_8(0xFF1B, 0xFF);
	mem_write_8(0xFF1C, 0x9F);
	mem_write_8(0xFF1E, 0xBF);
	mem_write_8(0xFF20, 0xFF);
	mem_write_8(0xFF21, 0x00);
	mem_write_8(0xFF22, 0x00);
	mem_write_8(0xFF23, 0xBF);
	mem_write_8(0xFF24, 0x77);
	mem_write_8(0xFF25, 0xF3);
	mem_write_8(0xFF26, 0xF1);
	mem_write_8(0xFF40, 0x91);
	mem_write_8(0xFF42, 0x00);
	mem_write_8(0xFF43, 0x00);
	mem_write_8(0xFF45, 0x00);
	mem_write_8(0xFF47, 0xFC);
	mem_write_8(0xFF48, 0xFF);
	mem_write_8(0xFF49, 0xFF);
	mem_write_8(0xFF4A, 0x00);
	mem_write_8(0xFF4B, 0x00);
	mem_write_8(0xFFFF, 0x00);    
		    
}

void dmg_init() {       	
	debug_init();
	cpu_init();

	if (skip_bios) {
		set_boot_state();
	} else {
		mem_enable_bios();
	}
	//cpu_test();
	//debug_run();
}

void dmg_reset() {
	printf("Not implemented yet\n");
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
