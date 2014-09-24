#include <SDL2/SDL.h>
#include <ctype.h>
#include <stdlib.h>

#define NCOLORS 4
#define WIDTH 160
#define HEIGHT 144

static uint8_t scale = 1;
static uint32_t palette[NCOLORS];

static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Surface *disp = NULL;
static SDL_Rect stretch_rect;
Uint32 rmask, gmask, bmask, amask;

void sdl_setup_palette() {
	palette[0] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	palette[1] = SDL_MapRGB(screen->format, 0x55, 0x55, 0x55);
	palette[2] = SDL_MapRGB(screen->format, 0xAA, 0xAA, 0xAA);
	palette[3] = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
}

int sdl_init() {

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	//Initialize all SDL subsystems
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		printf("%s\n", SDL_GetError());
		return 1;
	}
	window = SDL_CreateWindow("miniBoy by Dhole",
				  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOW_SHOWN,
				  WIDTH * scale, HEIGHT * scale, 
				  SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("%s\n", SDL_GetError());
		return 1;
	}
	screen = SDL_GetWindowSurface(window);
	disp = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32,
				    rmask, gmask, bmask, amask);
	stretch_rect.x = 0;
	stretch_rect.y = 0;
	stretch_rect.w = WIDTH * scale;
	stretch_rect.h = HEIGHT * scale;

	sdl_setup_palette();

	return 0;
}

void sdl_stop() {
	SDL_FreeSurface(disp);
	disp = NULL;
	SDL_FreeSurface(screen);
	screen = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_Quit();
}

void sdl_change_scale(uint8_t s) {
	scale = s;
	stretch_rect.w = WIDTH * scale;
	stretch_rect.h = HEIGHT * scale;
	SDL_SetWindowSize(window, WIDTH * scale, HEIGHT * scale);
	screen = SDL_GetWindowSurface(window);
}

void sdl_render(uint8_t *fb) {
	int i, j;
	SDL_Rect pixel = {0, 0, 1, 1};
	
	for (i = 0; i < HEIGHT; i++) {
		pixel.y = i;
		for (j = 0; j < WIDTH; j++) {
			pixel.x = j;
			SDL_FillRect(disp, &pixel, palette[fb[i*WIDTH + j]]);
		}
	}
       
	SDL_BlitScaled(disp, NULL, screen, &stretch_rect);
	SDL_UpdateWindowSurface(window);
}

void randomize(uint8_t *fb) {
	int i, j;
	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
		        fb[i*WIDTH + j] = rand() % 4;
		}
	}
}

int main() {
	sdl_init();
	sdl_setup_palette();

	uint8_t *test = calloc(WIDTH * HEIGHT, sizeof(uint8_t));

	int i, j;
	for (i = 0; i < HEIGHT; i++) {
		test[i * WIDTH + i] = 1;
	}
	test[100*WIDTH + 50] = 1;
	//dump_fb(test);
	//sdl_render(test);
	//SDL_Delay(1000);
	
	for (i = 0; i < 20; i++) {
		sdl_change_scale(i % 5 + 1);
		for (j = 0; j < 5; j++) {
			randomize(test);
			sdl_render(test);
			SDL_Delay(200);
		}
	}
	
	//sdl_change_scale(6);
	//sdl_render(test);
	//SDL_Delay(1000);
	//sdl_render(test);
	//SDL_UpdateWindowSurface(window);
	//SDL_Delay(1000);
	
	free(test);
	sdl_stop();
}
