#include <stdint.h>
#include <stdio.h>
#include <assert.h> // Temporary
#include "io_regs.h"
#include "screen.h"
#include "memory.h"

static uint8_t fb[160 * 144];
static uint8_t background[256 * 256];

static int32_t t_oam, t_oam_vram, t_hblank, t_vblank;
static uint8_t cur_line;

uint8_t *screen_get_fb() {
	return fb;
}

void screen_start_frame() {
	t_oam = SCREEN_TL_OAM;
	t_oam_vram = SCREEN_TL_OAM_VRAM;
	t_hblank = SCREEN_TL_HBLANK;
	t_vblank = SCREEN_TF_VBLANK;

	cur_line = SCREEN_LINES - 1;
}

void screen_write_fb() {
	int i, j;
	for (j = 0; j < SCREEN_SIZE_Y; j++) {
		for (i = 0; i < SCREEN_SIZE_X; i++) {
			fb[j * SCREEN_SIZE_X + i] = background[j * 256 + i];
		}
	}
}

void dump_background() {
	int i, j;
	// Dump just 1st quarter of screen 
	for (j = 0; j < 128; j++) {
		for (i = 0; i < 128; i++) {
			printf("%d", background[j * 256 + i]);
		}
		printf("\n");
	}
}

void screen_draw_line(uint8_t line) {
	uint16_t bg_tile_map, tile_data;
	uint8_t scroll_x, scroll_y; // scroll_y can't change during frame !!!
	uint8_t oam_row, obj, obj_line;
	uint8_t i, j;
	uint8_t obj_line_a, obj_line_b;
	
	// Check sprite size !!!
	// Background
	// If background enabled... !!!	
	
	scroll_x = mem_read_8(IO_SCROLLX);
	scroll_y = mem_read_8(IO_SCROLLY);
	switch (mem_bit_test(IO_LCDCONT,
			     MASK_IO_LCDCONT_BG_Title_Map_Display_Select)) {
	case OPT_BG_Tile_Map_Display_9800_9BFF:
		bg_tile_map = 0x9800;
		break;
	case OPT_BG_Tile_Map_Display_9C00_9FFF:
		bg_tile_map = 0x9C00;
		break;
	}
	switch (mem_bit_test(IO_LCDCONT,
			     MASK_IO_LCDCONT_BGWindow_Title_Data_Select)) {
	case OPT_BGWindow_Tile_Data_8800_97FF:
		// Some work to do here !!!
		tile_data = 0x9000;
		break;
	case OPT_BGWindow_Tile_Data_8000_8FFF:
		tile_data = 0x8000;
		break;
	}
	// WIP WIP WIP
	oam_row = (cur_line + scroll_y) / 8;
	obj_line = (cur_line + scroll_y) % 8;
	for (i = 0; i < 32; i++) {
		obj = mem_read_8(bg_tile_map + oam_row * 32 + i);
		obj_line_a = mem_read_8(tile_data + obj * 16 + obj_line * 2);
		obj_line_b = mem_read_8(tile_data + obj * 16 + obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			background[cur_line * 256 + (i * 8 + scroll_x + j) % 256] =
				((obj_line_a & (1 << (7 - j))) >> (7 - j)) +
				((obj_line_b & (1 << (7 - j))) >> (6 - j));
		}
	}

	/*
	if (scroll_y == 4 && cur_line == 100) {
		printf("bg_tile_map: 0x%04X\ntile_data: 0x%04X\n",
		       bg_tile_map, tile_data);
		dump_background();
		//assert(1 == 2);
	}
	*/
	/*
	if (cur_line == 100) {
		printf("Scroll y: %d\n", scroll_y);
	}
	*/
	
}

void screen_emulate(uint32_t cycles) {
	t_oam -= cycles;
	t_oam_vram -= cycles;
	t_hblank -= cycles;
	t_vblank -= cycles;

	if (mem_read_8(IO_CURLINE) != cur_line) {
		// !!! Reset register and LCD
	}

	// OAM mode 2
	if (t_oam <= 0) {
		cur_line = (cur_line + 1) % SCREEN_LINES;
		//printf("line: %d\n", cur_line);
		// update CURLINE REG
		mem_write_8(IO_CURLINE, cur_line);
		
		if (cur_line < SCREEN_SIZE_Y) {
			// Set Mode Flag to OAM at LCDSTAT
			mem_bit_unset(IO_LCDSTAT, MASK_IO_LCDSTAT_Mode_Flag);
			mem_bit_set(IO_LCDSTAT, OPT_Mode_OAM);
			// Interrupt OAM !!!
		}
		
		t_oam += SCREEN_DUR_LINE;
		
	}
	// OAM VRAM mode 3
	if (t_oam_vram <= 0) {
		if (cur_line < SCREEN_SIZE_Y) {
			// Set Mode Flag to OAM VRAM at LCDSTAT
			mem_bit_unset(IO_LCDSTAT, MASK_IO_LCDSTAT_Mode_Flag);
			mem_bit_set(IO_LCDSTAT, OPT_Mode_OAM_VRAM);
			//mem_bit_set(IO_IFLAGS, MASK_IO_INT_VBlank);
			// draw line
			screen_draw_line(cur_line);
		}
		t_oam_vram += SCREEN_DUR_LINE;;
	}
	// HBLANK mode 0
	if (t_hblank <= 0) {
		if (cur_line < SCREEN_SIZE_Y) {
			// Set Mode Flag to HBLANK at LCDSTAT
			mem_bit_unset(IO_LCDSTAT, MASK_IO_LCDSTAT_Mode_Flag);
			mem_bit_set(IO_LCDSTAT, OPT_Mode_HBlank);
			// Interrupt HBlank !!!
			//mem_bit_set(IO_IFLAGS, MASK_IO_INT_VBlank);
		}
		t_hblank += SCREEN_DUR_LINE;;
	}
	// VBLANK mode 1
	if (t_vblank <= 0) {
		// Set Mode Flag to VBLANK at LCDSTAT
		mem_bit_unset(IO_LCDSTAT, MASK_IO_LCDSTAT_Mode_Flag);
		mem_bit_set(IO_LCDSTAT, OPT_Mode_VBlank);
		// Interrupt VBlank
		mem_bit_set(IO_IFLAGS, MASK_IO_INT_VBlank);
		t_vblank += SCREEN_DUR_FRAME;
	}
}
