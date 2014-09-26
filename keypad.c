#include <stdlib.h>
#include <SDL2/SDL.h>

void keyboard_update_keys(uint8_t key[])
{
    const Uint8 *keystates = SDL_GetKeyboardState( NULL );
    key[0x0] = keystates[KEY_0];
    key[0x1] = keystates[KEY_1];
    key[0x2] = keystates[KEY_2];
    key[0x3] = keystates[KEY_3];
    key[0x4] = keystates[KEY_4];
    key[0x5] = keystates[KEY_5];
    key[0x6] = keystates[KEY_6];
    key[0x7] = keystates[KEY_7];
    key[0x8] = keystates[KEY_8];
    key[0x9] = keystates[KEY_9];
    key[0xA] = keystates[KEY_A];
    key[0xB] = keystates[KEY_B];
    key[0xC] = keystates[KEY_C];
    key[0xD] = keystates[KEY_D];
    key[0xE] = keystates[KEY_E];
    key[0xF] = keystates[KEY_F];
}
