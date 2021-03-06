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

static int t_oam, t_oam_vram, t_hblank, t_vblank;
static uint8_t cur_line, cmp_line;
static uint8_t reset;
static uint8_t bg_pal[4], obj0_pal[4], obj1_pal[4];
static uint8_t bgrdpal_reg, obj0pal_reg, obj1pal_reg;
static uint8_t lcdstat_reg;
static uint8_t scrollx_reg, scrolly_reg, wndposx_reg, wndposy_reg;

void set_pal(uint8_t *pal, uint8_t v) {
	pal[0] = v & 0x03;
	pal[1] = (v & 0x0C) >> 2;
	pal[2] = (v & 0x30) >> 4;
	pal[3] = (v & 0xC0) >> 6;
}

void screen_write_8(uint16_t addr, uint8_t v) {
	switch (addr) {
		case IO_CURLINE:
			reset = 1;
			break;
		case IO_CMPLINE:
			cmp_line = v;
			break;
		case IO_BGRDPAL:
			bgrdpal_reg = v;
			set_pal(bg_pal, v);
			break;
		case IO_OBJ0PAL:
			obj0pal_reg = v;
			set_pal(obj0_pal, v);
			obj0_pal[0] = 8; // Color 0 should be transparency
			break;
		case IO_OBJ1PAL:
			obj1pal_reg = v;
			set_pal(obj1_pal, v);
			obj1_pal[0] = 8; // Color 0 should be transparency
			break;
		case IO_SCROLLY:
			scrolly_reg = v;
			break;
		case IO_SCROLLX:
			//printf("Line %d, write scrollx: %d\n", cur_line, v);
			scrollx_reg = v;
			break;
		case IO_WNDPOSY:
			wndposy_reg = v;
			break;
		case IO_WNDPOSX:
			wndposx_reg = v;
			break;
		case IO_LCDSTAT:
			lcdstat_reg = (lcdstat_reg & 0x07) | (v & 0xF8);
		default:
			break;
	}
}

uint8_t screen_read_8(uint16_t addr) {
	switch (addr) {
		case IO_CURLINE:
			return cur_line;
			break;
		case IO_CMPLINE:
			return cmp_line;
			break;
		case IO_BGRDPAL:
			return bgrdpal_reg;
			break;
		case IO_OBJ0PAL:
			return obj0pal_reg;
			break;
		case IO_OBJ1PAL:
			return obj1pal_reg;
			break;
		case IO_SCROLLY:
			return scrolly_reg;
			break;
		case IO_SCROLLX:
			return scrollx_reg;
			break;
		case IO_WNDPOSY:
			return wndposy_reg;
			break;
		case IO_WNDPOSX:
			return wndposx_reg;
			break;
		case IO_LCDSTAT:
			return lcdstat_reg;
		default:
			break;
	}
	return 0;
}

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

	//cur_line = SCREEN_LINES - 1;
	cur_line = -1;
	// Set window and obj layers transparent
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			// Using value 8 as transparency
			bg_disp[j * 256 + 1] = 0;
			win_disp[j * 256 + i] = 8;
			obj_disp[j * 256 + i] = 8;
		}
	}
	screen_emulate(0);

}

void screen_draw_line_fb(uint8_t line) {
	int i;
	for (i = 0; i < SCREEN_SIZE_X; i++) {
		fb[line * SCREEN_SIZE_X + i] = bg_disp[line * 256 + i];
		// Add ordering between win and obj!
		// check non painted
		if (win_disp[line * 256 + i] < 8) {
			fb[line * SCREEN_SIZE_X + i] = win_disp[line * 256 + i];
		}
		// check transparency
		if (obj_disp[(line + SCREEN_SPRITE_INI_Y) * 256 +
				i + SCREEN_SPRITE_INI_X] < 8) {
			if (fb[line * SCREEN_SIZE_X + i] != 0 &&
					obj_disp[(line + SCREEN_SPRITE_INI_Y) * 256 +
					i + SCREEN_SPRITE_INI_X] >= 4) {
				continue;
			}
			fb[line * SCREEN_SIZE_X + i] =
				obj_disp[(line + SCREEN_SPRITE_INI_Y) * 256 +
				i + SCREEN_SPRITE_INI_X] & 0x03;
		}

	}
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
// scrolly_reg can't change during frame !!!
void screen_draw_line_bg(uint8_t line) {
	uint16_t bg_tile_map, tile_data;
	uint8_t oam_row, obj_line;
	uint8_t i, j;
	uint8_t obj_line_a, obj_line_b;
	int16_t obj;

	// Background

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
		// Some work to do here -> DONE
		tile_data = 0x9000;
		break;
	case OPT_BGWindow_Tile_Data_8000_8FFF:
		tile_data = 0x8000;
		break;
	}

	// WIP WIP WIP
	oam_row = (uint8_t)(line + scrolly_reg) / 8;
	obj_line = (uint8_t)(line + scrolly_reg) % 8;
	for (i = 0; i < 32; i++) {
		if (tile_data == 0x9000) {
			obj = (int8_t)mem_read_8(bg_tile_map + oam_row * 32 + i);
		} else {
			obj = (uint8_t)mem_read_8(bg_tile_map + oam_row * 32 + i);
		}
		obj_line_a = mem_read_8(tile_data + obj * 16 + obj_line * 2);
		obj_line_b = mem_read_8(tile_data + obj * 16 + obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			bg_disp[line * 256 + (uint8_t)(i * 8 - scrollx_reg + j)] =
				bg_pal[
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				((obj_line_b & (1 << (7 - j))) ? 2 : 0)
				];
		}
	}
}

void screen_draw_line_win(uint8_t line) {
	uint16_t win_tile_map, tile_data;
	uint8_t oam_row, obj_line;
	uint8_t i, j;
	uint8_t obj_line_a, obj_line_b;
	int16_t obj;

	// Window

	if (wndposy_reg > line || wndposx_reg > SCREEN_SIZE_X) {
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
		// Some work to do here -> DONE
		tile_data = 0x9000;
		break;
	case OPT_BGWindow_Tile_Data_8000_8FFF:
		tile_data = 0x8000;
		break;
	}

	// WIP WIP WIP
	oam_row = (uint8_t)(line - wndposy_reg) / 8;
	obj_line = (uint8_t)(line - wndposy_reg) % 8;
	for (i = 0; i < (SCREEN_SIZE_X - (wndposx_reg - 8)) / 8 + 1; i++) {
		if (tile_data == 0x9000) {
			obj = (int8_t)mem_read_8(win_tile_map + oam_row * 32 + i);
		} else {
			obj = (uint8_t)mem_read_8(win_tile_map + oam_row * 32 + i);
		}
		obj_line_a = mem_read_8(tile_data + obj * 16 + obj_line * 2);
		obj_line_b = mem_read_8(tile_data + obj * 16 + obj_line * 2 + 1);
		for (j = 0; j < 8; j++) {
			win_disp[line * 256 + (uint8_t)(i * 8 + wndposx_reg - 7 + j)] =
				bg_pal[
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				((obj_line_b & (1 << (7 - j))) ? 2 : 0)
				];
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
	uint16_t addr, pos;
	uint8_t obj_height, objs_line_len, obj_line;
	uint8_t obj_line_a, obj_line_b, color;
	uint8_t x_flip, y_flip, behind;
	uint8_t *pal;
	obj_t objs[40];
	obj_t *objs_line[40];

	line += SCREEN_SPRITE_INI_Y;

	switch (mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_OBJ_Size)) {
	case OPT_OBJ_Size_8x8:
		obj_height = 8;
		break;
	case OPT_OBJ_Size_8x16:
		//printf("8x16 objects not supported yet\n");
		//assert(1 == 2);
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
			/*
			printf("Object %d:\n", objs[i].id);
			printf("x: %02X, y: %02X, pat: %02X\n",
				objs[i].x, objs[i].y, objs[i].pat);
			printf("\n");
			*/
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

	for (i = first; i < objs_line_len; i++) {
		x_flip = (objs_line[i]->flags & OPT_OBJ_Flag_xflip) ? 1 : 0;
		y_flip = (objs_line[i]->flags & OPT_OBJ_Flag_yflip) ? 1 : 0;
		obj_line = (line - objs_line[i]->y) % obj_height;
		if (y_flip) {
			obj_line = obj_height - 1 - obj_line;
		}
		obj_line_a = mem_read_8(0x8000 + objs_line[i]->pat * 16 +
				obj_line * 2);
		obj_line_b = mem_read_8(0x8000 + objs_line[i]->pat * 16 +
				obj_line * 2 + 1);
		if (objs_line[i]->flags & OPT_OBJ_Flag_palette) {
			pal = obj1_pal;
		} else {
			pal = obj0_pal;
		}
		if (objs_line[i]->flags & OPT_OBJ_Flag_priority) {
			behind = 1;
		} else {
			behind = 0;
		}
		for (j = 0; j < 8; j++) {
			pos = line * 256 +
				(objs_line[i]->x + (x_flip ? 7 - j : j)) % 256;
			color = pal[
				((obj_line_a & (1 << (7 - j))) ? 1 : 0) +
				((obj_line_b & (1 << (7 - j))) ? 2 : 0)
				];
			if (color < 8) {
				obj_disp[pos] = color;
				if (behind) {
					obj_disp[pos] |= 4;
				}
			}
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

void bit_set(uint8_t *v, uint8_t bit) {
	*v |= bit;
}

void bit_unset(uint8_t *v, uint8_t bit) {
	*v &= ~bit;
}

int bit_test(uint8_t v, uint8_t bit) {
	return (v & bit) > 0;
}

int screen_emulate(uint32_t cycles) {
	// Makes Dr. Mario hang !!!
	if (!mem_bit_test(IO_LCDCONT, MASK_IO_LCDCONT_LCD_Display_Enable)) {
		reset = 1;
		return 0;
	}
	if (reset) {
		reset = 0;
		return 1;
	}

	t_oam -= cycles;
	t_oam_vram -= cycles;
	t_hblank -= cycles;
	t_vblank -= cycles;

	// OAM mode 2
	if (t_oam <= 0) {
		//printf("line: %d\n", cur_line);
		// update CURLINE REG
		cur_line = (cur_line + 1);// % SCREEN_LINES;
		if (cur_line == cmp_line) {
			bit_set(&lcdstat_reg, MASK_IO_LCDSTAT_Coincidence_Flag);
			if (bit_test(lcdstat_reg,
					MASK_IO_LCDSTAT_LYC_LY_Coincidence_Interrupt)) {
				mem_bit_set(IO_IFLAGS, MASK_IO_INT_LCDSTAT_Int);
			}
		} else {
			bit_unset(&lcdstat_reg, MASK_IO_LCDSTAT_Coincidence_Flag);
		}

		if (cur_line < SCREEN_SIZE_Y) {
			lcdstat_reg = (lcdstat_reg & 0xF3) | 0x02;
			// Set Mode Flag to OAM at LCDSTAT
			bit_unset(&lcdstat_reg, MASK_IO_LCDSTAT_Mode_Flag);
			bit_set(&lcdstat_reg, OPT_Mode_OAM);
			// Interrupt OAM
			if (bit_test(lcdstat_reg,
					MASK_IO_LCDSTAT_Mode_2_OAM_Interrupt)) {
				mem_bit_set(IO_IFLAGS, MASK_IO_INT_LCDSTAT_Int);
			}
		}

		t_oam += SCREEN_DUR_LINE;

	}
	// OAM VRAM mode 3
	if (t_oam_vram <= 0) {
		if (cur_line < SCREEN_SIZE_Y) {
			lcdstat_reg = (lcdstat_reg & 0xF3) | 0x03;
			// Set Mode Flag to OAM VRAM at LCDSTAT
			bit_unset(&lcdstat_reg, MASK_IO_LCDSTAT_Mode_Flag);
			bit_set(&lcdstat_reg, OPT_Mode_OAM_VRAM);
		}
		t_oam_vram += SCREEN_DUR_LINE;
	}
	// HBLANK mode 0
	if (t_hblank <= 0) {
		if (cur_line < SCREEN_SIZE_Y) {
			lcdstat_reg = (lcdstat_reg & 0xF3) | 0x00;
			// Set Mode Flag to HBLANK at LCDSTAT
			bit_unset(&lcdstat_reg, MASK_IO_LCDSTAT_Mode_Flag);
			bit_set(&lcdstat_reg, OPT_Mode_HBlank);
			// Interrupt HBlank
			if (bit_test(lcdstat_reg,
					MASK_IO_LCDSTAT_Mode_0_HBlank_Interrupt)) {
				mem_bit_set(IO_IFLAGS, MASK_IO_INT_LCDSTAT_Int);
			}
			// draw line
			screen_draw_line(cur_line);
		}
		t_hblank += SCREEN_DUR_LINE;
	}
	// VBLANK mode 1
	if (t_vblank <= 0) {
		lcdstat_reg = (lcdstat_reg & 0xF3) | 0x01;
		// Set Mode Flag to VBLANK at LCDSTAT
		bit_unset(&lcdstat_reg, MASK_IO_LCDSTAT_Mode_Flag);
		bit_set(&lcdstat_reg, OPT_Mode_VBlank);
		// Interrupt VBlank
		mem_bit_set(IO_IFLAGS, MASK_IO_INT_VBlank);
		if (bit_test(lcdstat_reg,
				MASK_IO_LCDSTAT_Mode_1_VBlank_Interrupt)) {
			mem_bit_set(IO_IFLAGS, MASK_IO_INT_LCDSTAT_Int);
		}
		t_vblank += SCREEN_DUR_FRAME;
	}
	return 0;
}
