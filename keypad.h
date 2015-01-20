
typedef enum {
	KEYPAD_DOWN,
	KEYPAD_UP,
	KEYPAD_LEFT,
	KEYPAD_RIGHT,
	KEYPAD_START,
	KEYPAD_SELECT,
	KEYPAD_B,
	KEYPAD_A
} keypad_t;

uint8_t keypad_read_8(uint16_t addr);
void keypad_write_8(uint16_t addr, uint8_t v);
void keypad_emulate(uint8_t *keypad);
