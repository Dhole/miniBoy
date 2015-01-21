#define SCREEN_SPRITE_INI_X 8
#define SCREEN_SPRITE_INI_Y 16
#define SCREEN_SIZE_X 160
#define SCREEN_SIZE_Y 144
#define SCREEN_SPRITE_END_X SCREEN_SPRITE_INI_X + SCREEN_SIZE_X
#define SCREEN_SPRITE_END_Y SCREEN_SPRITE_INI_Y + SCREEN_SIZE_Y

#define SCREEN_LINES 154

// Duration of each screen state
#define SCREEN_DUR_FRAME 70224
#define SCREEN_DUR_OAM 80
#define SCREEN_DUR_OAM_VRAM 172
#define SCREEN_DUR_HBLANK 204
#define SCREEN_DUR_LINE 456 // 80 + 172 + 204
#define SCREEN_DUR_VBLANK 4560

// Start of each screen state in each line / frame
#define SCREEN_TL_OAM 0
#define SCREEN_TL_OAM_VRAM SCREEN_TL_OAM + SCREEN_DUR_OAM
#define SCREEN_TL_HBLANK SCREEN_TL_OAM_VRAM + SCREEN_DUR_OAM_VRAM
#define SCREEN_TF_VBLANK SCREEN_DUR_LINE * SCREEN_SIZE_Y

#define SCREEN_FPS 59.73

#define MASK_OBJ_PRIORITY 0x01 << 7
#define MASK_OBJ_Y_FLIP   0x01 << 6
#define MASK_OBJ_X_FLIP   0x01 << 5
#define MASK_OBJ_PALETTE  0x01 << 4

typedef struct {
	uint8_t id;
	uint8_t x;
	uint8_t y;
	uint8_t pat;
	uint8_t flags;
} obj_t;

uint8_t *screen_get_fb();
//void screen_write_fb();
void screen_start_frame();
int screen_emulate(uint32_t cycles);
void screen_write_8(uint16_t addr, uint8_t v);
uint8_t screen_read_8(uint16_t addr);
