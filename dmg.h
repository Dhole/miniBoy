
void dmg_load_bios(char *bios_path);
void dmg_load_rom(char *rom_path);
void dmg_unload_rom();
void dmg_init();
void dmg_reset();
uint32_t dmg_run(uint32_t delta, uint8_t *keypad, int *debug_flag, int *debug_pause);

