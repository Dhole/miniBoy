#include <SDL2/SDL.h>

// Key mapping

// GameBoy keys
#define KEY_A SDL_SCANCODE_Z
#define KEY_B SDL_SCANCODE_X
#define KEY_U SDL_SCANCODE_UP
#define KEY_D SDL_SCANCODE_DOWN
#define KEY_L SDL_SCANCODE_LEFT
#define KEY_R SDL_SCANCODE_RIGHT
#define KEY_SELECT SDL_SCANCODE_BACKSPACE
#define KEY_START SDL_SCANCODE_ENTER

void keyboard_update_keys(uint8_t key[]);
