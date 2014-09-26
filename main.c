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
int debug_flag;

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
				break;
			case KEY_SCALE_1:
				if (scale > 1) {
					scale /= 2;
					sdl_change_scale(scale);
				}
				break;
			case KEY_SCALE_2:
				if (scale < 8) {
					scale *= 2;
					sdl_change_scale(scale);
				}
				break;
			case KEY_EXIT:
			        return E_EXIT;
				break;
			default:
				break;
			}
		}
	}
	return E_NOTHING;
}

int main_loop(Uint32 time) {
	SDL_Event e;
	// Capture events
	while (SDL_PollEvent(&e)) {
		switch(process_event(&Keye)) {
		case E_RESET:
			dmg_reset();
			break;
		case E_EXIT:
			return 1;
		default:
			break;
		}
	}
	if (!pause_emu) {
		//keyboard_update_keys();
		// Do sound or whatever
		// ...
		dmg_run(debug_flag);
	}
	return 0;
}

int main(int argc, char** argv) {
	int opt;
	//int verbose_flag = 0;
	char *bios_path = NULL;
	char *rom_path = NULL;
	Uint32 start_time, time, frame;

	pause_emu = 0;

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
			break;
		default:
		        usage(argv[0]);
		}
	}
	if (optind == argc) {
		usage(argv[0]);
	}
	rom_path = argv[optind];

	dmg_load_bios(bios_path);
	dmg_load_rom(rom_path);
	dmg_init();
	
	sdl_init();

	frame = 1;
	start_time = SDL_GetTicks();
	for (;;) {
		time = (frame * 1000) / SCREEN_FPS;
		
		if (main_loop(time)) {
			break;
		}

		sdl_render(screen_get_fb());
		
	        // Sync to maximum fps
		if (SDL_GetTicks() - start_time < time)  {
			SDL_Delay(time - (SDL_GetTicks() - start_time));
		} else {
			frame = 0;
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
