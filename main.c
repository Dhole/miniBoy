#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "main.h"
#include "dmg.h"
#include "screen.h"
#include "keypad.h"
#include "sdl.h"

Uint32 start_time, last_time, elapsed_time;
Uint32 scale;
int pause_emu;
int debug_flag, debug_pause;
uint8_t keypad[8] = {0,0,0,0,0,0,0,0};

void usage(char *argv0) {
	printf("Usage: %s [-v] [-d] [-b bios] rom\n", argv0);
	exit(1);
}

event_t process_event(SDL_Event *e) {
	if (e->type == SDL_QUIT) {
		return 1;
	}
	if (!e->key.repeat) {
		if (e->type == SDL_KEYDOWN) {
			switch (e->key.keysym.sym) {
			case KEY_PAUSE:
				pause_emu = pause_emu ^ 1;
				break;
			case KEY_RESET:
				return E_RESET;
				break;
			case KEY_ENTER_DEBUG:
			        debug_flag = 1;
				debug_pause = 1;
				break;
			case KEY_SCALE_1:
				if (scale > 1) {
					scale -= 1;
					sdl_change_scale(scale);
				}
				break;
			case KEY_SCALE_2:
				if (scale < 6) {
					scale += 1;
					sdl_change_scale(scale);
				}
				break;
			case KEY_EXIT:
			        return E_EXIT;
				break;
			// GameBoy Keys
			case KEY_DOWN:
				keypad[KEYPAD_DOWN] = 1;
				break;
			case KEY_UP:
				keypad[KEYPAD_UP] = 1;
				break;
			case KEY_LEFT:
				keypad[KEYPAD_LEFT] = 1;
				break;
			case KEY_RIGHT:
				keypad[KEYPAD_RIGHT] = 1;
				break;
			case KEY_START:
				keypad[KEYPAD_START] = 1;
				break;
			case KEY_SELECT:
				keypad[KEYPAD_SELECT] = 1;
				break;
			case KEY_B:
				keypad[KEYPAD_B] = 1;
				break;
			case KEY_A:
				keypad[KEYPAD_A] = 1;
				break;
			default:
				break;
			}
		} else if (e->type == SDL_KEYUP) {
			switch (e->key.keysym.sym) {
			// GameBoy Keys
			case KEY_DOWN:
				keypad[KEYPAD_DOWN] = 0;
				break;
			case KEY_UP:
				keypad[KEYPAD_UP] = 0;
				break;
			case KEY_LEFT:
				keypad[KEYPAD_LEFT] = 0;
				break;
			case KEY_RIGHT:
				keypad[KEYPAD_RIGHT] = 0;
				break;
			case KEY_START:
				keypad[KEYPAD_START] = 0;
				break;
			case KEY_SELECT:
				keypad[KEYPAD_SELECT] = 0;
				break;
			case KEY_B:
				keypad[KEYPAD_B] = 0;
				break;
			case KEY_A:
				keypad[KEYPAD_A] = 0;
				break;
			default:
				break;
			}
		}
	return E_NOTHING;
}

int main_loop(uint32_t delta) {
	SDL_Event e;
	uint32_t elapsed = delta;
	// Capture events
	while (SDL_PollEvent(&e)) {
		switch(process_event(&e)) {
		case E_RESET:
			dmg_reset();
			break;
		case E_EXIT:
			return -1;
			break;
		default:
			break;
		}
	}
	if (!pause_emu) {
		//keyboard_update_keys();
		// Do sound or whatever
		// ...
		elapsed = dmg_run(delta, &debug_flag, &debug_pause);

		// Something
		//randomize(screen_get_fb());
	}
	return elapsed;
}

int main(int argc, char** argv) {
	int opt;
	//int verbose_flag = 0;
	char *bios_path = NULL;
	char *rom_path = NULL;
	Uint32 start_time, time, frame;
	uint32_t delta, elapsed;

	pause_emu = 0;
	scale = 2;
	
	while ((opt = getopt(argc, argv, "vdb:")) != -1) {
		switch (opt) {
		case 'b':
			bios_path = optarg;
			break;
		case 'v':
			//verbose_flag = 1;
			break;
		case 'd':
			debug_flag = 1;
			debug_pause = 1;
			break;
		default:
		        usage(argv[0]);
		}
	}
	if (optind == argc) {
		usage(argv[0]);
	}
	rom_path = argv[optind];

	dmg_init();

	dmg_load_bios(bios_path);
	dmg_load_rom(rom_path);
	
	sdl_init(scale);

	time = 0;
	frame = 1;
	start_time = SDL_GetTicks();
	for (;;) {
		delta = ((frame * 1000) / SCREEN_FPS) - time;
		
		if ((elapsed = main_loop(delta)) == -1) {
			break;
		}
		time += elapsed;
		
		if (elapsed > 0) {
			sdl_render(screen_get_fb());
		}
		
	        // Sync to maximum fps
		if (SDL_GetTicks() - start_time < time)  {
			SDL_Delay(time - (SDL_GetTicks() - start_time));
		} else {
			time = 0;
			frame = 1;
			start_time = SDL_GetTicks();
		}
		frame++;
		// When Uint32 overflows, restart. This is unlikely to happen:
		// Uint32 will overflow after 828 days at 60 fps
		if (frame == 0) {
			start_time = SDL_GetTicks();
		}
	}

	sdl_stop();

	return 0;
}
