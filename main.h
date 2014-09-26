// Function keys
#define KEY_PAUSE SDLK_p
#define KEY_RESET SDLK_r
#define KEY_ENTER_DEBUG SDLK_d
#define KEY_SCALE_1 SDLK_1
#define KEY_SCALE_2 SDLK_2
#define KEY_EXIT SDLK_ESCAPE

typedef enum {
	E_RESET,
	E_EXIT,
	E_NOTHING
} event_t;

event_t process_event(SDL_Event *e);
