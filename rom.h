#define MAX_ROM_SIZE 0x400000
#define MIN_ROM_SIZE 0x8000

#define ROM_POS_TITLE 0x0143
#define ROM_POS_LICENSE_NEW_H 0x0144
#define ROM_POS_LICENSE_NEW_L 0x0145
#define ROM_POS_SGB 0x0146
#define ROM_POS_CART_TYPE 0x0147
#define ROM_POS_ROM_SIZE 0x0148
#define ROM_POS_RAM_SIZE 0x0149
#define ROM_POS_COUNTRY 0x014A
#define ROM_POS_LICENSE_OLD 0x014B
#define ROM_POS_ROM_VER 0x014C
#define ROM_POS_COMP_CHECK 0x014D
#define ROM_POS_CHECKSUM 0x014E

#define ROM_TITLE_LEN 15

typedef enum {
	ROM_ONLY                  = 0x00,
	ROM_MBC1                  = 0x01,
	ROM_MBC1_RAM              = 0x02,
	ROM_MBC1_RAM_BATT         = 0x03,
	ROM_MBC2                  = 0x05,
	ROM_MBC2_BATT             = 0x06,
	ROM_RAM                   = 0x08,
	ROM_RAM_BATT              = 0x09,
	ROM_MMM01                 = 0x0B,
	ROM_MMM01_SRAM            = 0x0C,
	ROM_MMM01_SRAM_BATT       = 0x0D,
	ROM_MBC3_TIMER_BATT       = 0x0F,
	ROM_MBC3_TIMER_RAM_BATT   = 0x10,
	ROM_MBC3                  = 0x11,
	ROM_MBC3_RAM              = 0x12,
	ROM_MBC3_RAM_BATT         = 0x13,
	ROM_MBC5                  = 0x19,
	ROM_MBC5_RAM              = 0x1A,
	ROM_MBC5_RAM_BATT         = 0x1B,
	ROM_MBC5_RUMBLE           = 0x1C,
	ROM_MBC5_RUMB_E_SRAM      = 0x1D,
	ROM_MBC5_RUMBLE_SRAM_BATT = 0x1E,
	ROM_PocketCamera          = 0x1F,
	ROM_BandaiTAMA5           = 0xFD,
	ROM_HudsonHuC3            = 0xFE,
	ROM_HudsonHuC1            = 0xFF,
} cart_type_t;

typedef enum {
	ROM_256Kbit = 0x00,
	ROM_512Kbit = 0x01,
	ROM_1Mbit   = 0x02,
	ROM_2Mbit   = 0x03,
	ROM_4Mbit   = 0x04,
	ROM_8Mbit   = 0x05,
	ROM_16Mbit  = 0x06,
	ROM_9Mbit   = 0x52,
	ROM_10Mbit  = 0x53,
	ROM_12Mbit  = 0x54,
} rom_size_t;

typedef enum {
	RAM_NONE    = 0x00,
	RAM_16Kbit  = 0x01,
	RAM_64Kbit  = 0x02,
	RAM_256Kbit = 0x03,
	RAM_1Mbit   = 0x04,
} ram_size_t;

typedef enum {
	MBC_NONE,
	MBC_1,
	MBC_2,
	MBC_3,
	MBC_5,
} mbc_t;

typedef struct {
	char title[16];
	//cart_compat_t compat;
	char license_new[2];
	char super_gameboy;
	cart_type_t type;
	mbc_t mbc;
	unsigned int rom_size;
	unsigned int ram_size;
	char region;
	char license_old;
	char rom_ver;
	char complement_check;
	char checksum[2];
} rom_info_t;

void rom_init();
void rom_load(char *rom_path);
void rom_unload();
int rom_read_info();
void rom_print_info();
uint8_t rom_read_8(uint16_t addr);
void rom_write_8(uint16_t addr, uint8_t v);
