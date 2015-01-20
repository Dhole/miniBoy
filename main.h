// Function keys
#define KEY_PAUSE SDLK_p
#define KEY_RESET SDLK_r
#define KEY_ENTER_DEBUG SDLK_d
#define KEY_SCALE_1 SDLK_1
#define KEY_SCALE_2 SDLK_2
#define KEY_EXIT SDLK_ESCAPE

// Key mapping

// GameBoy keys
#define KEY_A SDLK_z
#define KEY_B SDLK_x
#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_SELECT SDLK_BACKSPACE
#define KEY_START SDLK_RETURN

typedef enum {
	E_RESET,
	E_EXIT,
	E_NOTHING
} event_t;

event_t process_event(SDL_Event *e);
