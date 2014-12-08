#include <stdint.h>
#include <stdio.h>
#include <assert.h> // Temporary
#include "io_regs.h"
#include "screen.h"
#include "memory.h"
#include "insertion_sort.h"

static uint8_t fb[160 * 144];
static uint8_t bg_disp[256 * 256];
static uint8_t win_disp[256 * 256];
static uint8_t obj_disp[256 * 256];

static int32_t t_oam, t_oam_vram, t_hblank, t_vblank;
static uint8_t cur_line;


uint8_t *screen_get_fb() {
	return fb;
}

uint8_t *screen_get_bg() {
	return bg_disp;
}

uint8_t *screen_get_win() {
	return win_disp;
}

uint8_t *screen_get_obj() {
	return obj_disp;
}

void screen_start_frame() {
	int i, j;
	t_oam = SCREEN_TL_OAM;
	t_oam_vram = SCREEN_TL_OAM_VRAM;
	t_hblank = SCREEN_TL_HBLANK;
	t_vblank = SCREEN_TF_VBLANK;

	cur_line = SCREEN_LINES - 1;
	// Set window and obj layers transparent
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			bg_disp[j * 256 + 1] = 4;
			win_disp[j * 256 + i] = 5;
			obj_disp[j * 256 + i] = 0;
		}
	}

}

/*
void screen_write_fb() {
	int i, j;
	if (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_BG_Display_Enable)) {
		for (j = 0; j < SCREEN_SIZE_Y; j++) {
			for (i = 0; i < SCREEN_SIZE_X; i++) {
				fb[j * SCREEN_SIZE_X + i] = bg_disp[j * 256 + i];
			}
		}
	}
	if (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_Window_Display_Enable)) {
		for (j = 0; j < SCREEN_SIZE_Y; j++) {
			for (i = 0; i < SCREEN_SIZE_X; i++) {
				fb[j * SCREEN_SIZE_X + i] = win_disp[j * 160 + i];
			}
		}
	}
	if (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_OBJ_Display_Enable)) {
		for (j = 0; j < SCREEN_SIZE_Y; j++) {
			for (i = 0; i < SCREEN_SIZE_X; i++) {
				fb[j * SCREEN_SIZE_X + i] = obj_disp[j * 160 + i];
			}
		}
	}
}
*/

void screen_draw_line_fb(uint8_t line) {
	int i;
	for (i = 0; i < SCREEN_SIZE_X; i++) {
		fb[line * SCREEN_SIZE_X + i] = bg_disp[line * 256 + i];
		// Add ordering between win and obj!
		// check non painted
		if (win_disp[line * 256 + i] < 4) { 
			fb[line * SCREEN_SIZE_X + i] = win_disp[line * 256 + i];
		}
		// check transparency
		if (obj_disp[(line + SCREEN_SPRITE_INI_Y) * 256 +
			     i + SCREEN_SPRITE_INI_X] != 0) { 
			fb[line * SCREEN_SIZE_X + i] =
				obj_disp[(line + SCREEN_SPRITE_INI_Y) * 256 +
				i + SCREEN_SPRITE_INI_X];
		}
		
	}
	// Apply palette at end of each draw_line_{bg, win, obj} !!!
}

void dump_some_layer() {
	int i, j;
	// Dump just 1st quarter of screen 
	for (j = 0; j < 64; j++) {
		for (i = 0; i < 128; i++) {
			printf("%d", win_disp[j * 256 + i]);
		}
		printf("\n");
	}
	printf("\n");
}

void screen_draw_line_bg(uint8_t line) {
	uint16_t bg_tile_map, tile_data;
	uint8_t scroll_x, scroll_y; // scroll_y can't change during frame !!!
	uint8_t oam_row, obj_line;
	uint8_t i, j;
	uint8_t obj_line_a, obj_line_b;
	int16_t obj;
	
	// Background
	
	scroll_x = mem_read_8(IO_SCROLLX);
	scroll_y = mem_read_8(IO_SCROLLY);
	switch (mem_bit_test(IO_LCDCONT,
			     MASK_IO_LCDCONT_BG_Tile_Map_Display_Select)) {
	case OPT_BG_Tile_Map_9800_9BFF:
		bg_tile_map = 0x9800;
		break;
	case OPT_BG_Tile_Map_9C00_9FFF:
		bg_tile_map = 0x9C00;
		break;
	}
	switch (mem_bit_test(IO_LCDCONT,
			     MASK_IO_LCDCONT_BGWindow_Tile_Data_Select)) {
	case OPT_BGWindow_Tile_Data_8800_97FF:
		// Some work to do here !!!
		tile_data = 0x9000;
		break;
	case OPT_BGWindow_Tile_Data_8000_8FFF:
		tile_data = 0x8000;
		break;
	}
	
	// WIP WIP WIP
	oam_row = (line + scroll_y) / 8;
	obj_line = (line + scroll_y) % 8;
	for (i = 0; i < 32; i++) {
		if (tile_data == 0x9000) {
			obj = (int8_t)mem_read_8(bg_tile_map + oam_row * 32 + i);
		} else {
			obj = (uint8_t)mem_read_8(bg_tile_map + oam_row * 32 + i);
		}
		obj_line_a = mem_read_8(tile_data + obj * 16 + obj_line * 2);
		obj_line_b = mem_read_8(tile_data + obj * 16 + obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			bg_disp[line * 256 + (i * 8 + scroll_x + j) % 256] =
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				((obj_line_b & (1 << (7 - j))) ? 2 : 0);
		}
	}
}

void screen_draw_line_win(uint8_t line) {
	uint16_t win_tile_map, tile_data;
	uint8_t pos_x, pos_y; // scroll_y can't change during frame !!!
	uint8_t oam_row, obj_line;
	uint8_t i, j;
	uint8_t obj_line_a, obj_line_b;
	int16_t obj;
	
	// Window
	
	pos_x = mem_read_8(IO_SCROLLX) - 7;
	pos_y = mem_read_8(IO_SCROLLY);
	if (pos_y > line || pos_x > SCREEN_SIZE_X) {
		return;
	}
	switch (mem_bit_test(IO_LCDCONT,
			     MASK_IO_LCDCONT_Window_Tile_Map_Display_Select)) {
	case OPT_Window_Tile_Map_9800_9BFF:
		win_tile_map = 0x9800;
		break;
	case OPT_Window_Tile_Map_9C00_9FFF:
		win_tile_map = 0x9C00;
		break;
	}
	switch (mem_bit_test(IO_LCDCONT,
			     MASK_IO_LCDCONT_BGWindow_Tile_Data_Select)) {
	case OPT_BGWindow_Tile_Data_8800_97FF:
		// Some work to do here !!!
		tile_data = 0x9000;
		break;
	case OPT_BGWindow_Tile_Data_8000_8FFF:
		tile_data = 0x8000;
		break;
	}
	
	// WIP WIP WIP
	oam_row = (line - pos_y) / 8;
	obj_line = (line - pos_y) % 8;
	for (i = 0; i < (SCREEN_SIZE_X - pos_x + 7) / 8; i++) {
		if (tile_data == 0x9000) {
			obj = (int8_t)mem_read_8(win_tile_map + oam_row * 32 + i);
		} else {
			obj = (uint8_t)mem_read_8(win_tile_map + oam_row * 32 + i);
		}
		obj_line_a = mem_read_8(tile_data + obj * 16 + obj_line * 2);
		obj_line_b = mem_read_8(tile_data + obj * 16 + obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			win_disp[line * 256 + (i * 8 + pos_x + j) % 256] =
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				((obj_line_b & (1 << (7 - j))) ? 2 : 0);
		}
	}
}
// If a < b -> 1, else if a > b -> 0
int obj_comp(void *array, int i, int j) {
	obj_t *a = ((obj_t**)array)[i];
	obj_t *b = ((obj_t**)array)[j];
	// Compare x position
	if (a->x < b->x) {
		return 0;
	}
	if (a->x > b->x) {
		return 1;
	}
	// Compare order in table
	if (a->id < b->id) {
		return 0;
	}
	if (a->id > b->id) {
		return 1;
	}
	// This should never be reached
	return 0;
}

void obj_swap(void *array, int i, int j) {
	obj_t *a = ((obj_t**)array)[i];
	obj_t *b = ((obj_t**)array)[j];

	((obj_t**)array)[i] = b;
	((obj_t**)array)[j] = a;
}

void screen_draw_line_obj(uint8_t line) {
	// 8x8 or 8x16
	// Object Patern Table = 8000 - 8FFF
	// Object Atribute Table = FE00 - FE9F
	// 10 sprites per scan line
	// higher X -> draw first (lower X -> draw above others)
	// same X -> FE00 highest, FE04 next highest
	// Y = 0 or Y => 144+16, discard sprite

	int i, j, first;
	uint16_t addr;
	uint8_t obj_height, objs_line_len, obj_line;
	uint8_t obj_line_a, obj_line_b;
	obj_t objs[40];
	obj_t *objs_line[40];

	line += SCREEN_SPRITE_INI_Y;

	switch (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_OBJ_Size)) {
	case OPT_OBJ_Size_8x8:
		obj_height = 8;
		break;
	case OPT_OBJ_Size_8x16:
		obj_height = 16;
		break;
	}
	// Read all the obj attributes
	for (i = 0; i < 40; i++) {
		addr = MEM_OAM + i * 4;

		objs[i].id = i;
		objs[i].y = mem_read_8(addr++);
		objs[i].x = mem_read_8(addr++);
		objs[i].pat = mem_read_8(addr++);
		objs[i].flags = mem_read_8(addr);
		//printf("x: %02X, y: %02X - ", objs[i].x, objs[i].y);
	}
	//printf("\n");

	// Take the candidate objects to be drawn in the line
	objs_line_len = 0;
	for (i = 0; i < 40; i++) {
		if((objs[i].y != 0) && (objs[i].y < SCREEN_SPRITE_END_Y) &&
		   (objs[i].y <= line) && ((objs[i].y + obj_height) > line)) {
			objs_line[objs_line_len++] = &objs[i];
			//printf("Object %d:\n", objs[i].id);
			//printf("x: %02X, y: %02X, pat: %02X\n",
			//       objs[i].x, objs[i].y, objs[i].pat);
			//return;
		}
	}

	// Sort the candidate objects by priority
	insertion_sort(objs_line, objs_line_len, obj_comp, obj_swap);

	// Draw objects by order of priority
	if (objs_line_len > 10) {
		first = objs_line_len - 10;
	} else {
		first = 0;
	}

	// Only taking into account 8x8!!!
	for (i = first; i < objs_line_len; i++) {
		obj_line = (line - objs_line[i]->y) % obj_height;
		obj_line_a = mem_read_8(0x8000 + objs_line[i]->pat * 16 +
					obj_line * 2);
		obj_line_b = mem_read_8(0x8000 + objs_line[i]->pat * 16 +
					obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			obj_disp[line * 256 + (objs_line[i]->x + j) % 256] =
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				((obj_line_b & (1 << (7 - j))) ? 2 : 0);
		}
	}
}

void screen_draw_line(uint8_t line) {
	if (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_BG_Display_Enable)) {
		screen_draw_line_bg(line);
	}
	if (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_Window_Display_Enable)) {
		screen_draw_line_win(line);
	}
	if (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_OBJ_Display_Enable)) {
		screen_draw_line_obj(line);
	}

	screen_draw_line_fb(line);
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
