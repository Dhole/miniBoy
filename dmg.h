#define CPU_FREQ 4194304

void dmg_load_bios(char *bios_path);
void dmg_load_rom(char *rom_path);
void dmg_unload_rom();
void dmg_init();
void dmg_reset();
void dmg_run(int debug_flag);
