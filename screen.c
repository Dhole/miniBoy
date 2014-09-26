#include <stdint.h>
#include "screen.h"
#include "memory.h"

static uint8_t fb[160 * 144];

uint8_t *screen_get_fb() {
	return fb;
}
