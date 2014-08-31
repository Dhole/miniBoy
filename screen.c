

#define SCREEN_INI_X 8
#define SCREEN_INI_Y 16
#define SCREEN_SIZE_X 160
#define SCREEN_SIZE_Y 144
#define SCREEN_END_X SCREEN_INI_X + SCREEN_SIZE_X
#define SCREEN_END_Y SCREEN_INI_Y + SCREEN_SIZE_Y

#define OFF 0
#define ON  1

// IO Ports

#define IO_JOYPAD  0xFF00
#define IO_SIODATA 0xFF01
#define IO_SIOCONT 0xFF02
#define IO_DIVIDER 0xFF04
#define IO_TIMECNT 0xFF05
#define IO_TIMEMOD 0xFF06
#define IO_TIMCONT 0xFF07
#define IO_IFLAGS  0xFF0F
// Sound registers
#define IO_SNDREG10 0xFF10
#define IO_SNDREG11 0xFF12
#define IO_SNDREG12 0xFF13
#define IO_SNDREG13 0xFF14
#define IO_SNDREG14 0xFF15
#define IO_SNDREG21 0xFF16
#define IO_SNDREG22 0xFF17
#define IO_SNDREG23 0xFF18
#define IO_SNDREG24 0xFF19
#define IO_SNDREG30 0xFF1A
#define IO_SNDREG31 0xFF1B
#define IO_SNDREG32 0xFF1C
#define IO_SNDREG33 0xFF1D
#define IO_SNDREG34 0xFF1E

#define IO_SNDREG41 0xFF20
#define IO_SNDREG42 0xFF21
#define IO_SNDREG43 0xFF22
#define IO_SNDREG44 0xFF23
#define IO_SNDREG50 0xFF24
#define IO_SNDREG51 0xFF25
#define IO_SNDREG52 0xFF26

// Screen registers
#define IO_LCDCONT 0xFF40
#define IO_LCDSTAT 0xFF41
#define IO_SCROLLY 0xFF42
#define IO_SCROLLX 0xFF43
#define IO_CURLINE 0xFF44
#define IO_CMPLINE 0xFF45

#define IO_BGRDPAL 0xFF47
#define IO_OBJ0PAL 0xFF48
#define IO_OBJ1PAL 0xFF49
#define IO_WNDPOSY 0xFF4A
#define IO_WNDPOSX 0xFF4B

#define IO_DMACONT 0xFF46
#define IO_ISWITCH 0xFFFF

#define MASK_IO_LCDCONT_LCD_Display_Enable             0x01 << 7
#define MASK_IO_LCDCONT_Window_Tile_Map_Display_Select 0x01 << 6
#define MASK_IO_LCDCONT_Window_Display_Enable          0x01 << 5
#define MASK_IO_LCDCONT_BGWindow_Title_Data_Select     0x01 << 4
#define MASK_IO_LCDCONT_BG_Title_Map_Display_Select    0x01 << 3
#define MASK_IO_LCDCONT_OBJ_Size                       0x01 << 2
#define MASK_IO_LCDCONT_OBJ_Display_Enable             0x01 << 1
#define MASK_IO_LCDCONT_BG_Display                     0x01 << 0

#define OPT_Window_Tile_Map_9800_9BFF     0
#define OPT_Window_Tile_Map_9C00_9FFF     1
#define OPT_BGWindow_Tile_Data_8800_97FF  0
#define OPT_BGWindow_Tile_Data_8000_8FFF  1
#define OPT_BG_Tile_Map_Display_9800_9BFF 0
#define OPT_BG_Tile_Map_Display_9C00_9FFF 1

#define MASK_IO_LCDSTAT_LYC_LY_Coincidence_Interrupt 0x01 << 6
#define MASK_IO_LCDSTAT_Mode_2_OAM_Interrupt         0x01 << 5
#define MASK_IO_LCDSTAT_Mode_1_VBlank_Interrupt      0x01 << 4
#define MASK_IO_LCDSTAT_Mode_0_HBlank_Interrupt      0x01 << 3
#define MASK_IO_LCDSTAT_Coincidence_Flag             0x01 << 2
#define MASK_IO_LCDSTAT_Mode_Flag                    0x01 << 1 + 0x01 << 0

#define OPT_LYC_neq_LY 0
#define OPT_LYC_eq_LY  1
#define OPT_During_HBlank             0
#define OPT_During_VBlank             1
#define OPT_During_Searching_OAM      2
#define OPT_During_Transf_Data_to_LCD 3

#define MASK_IO_BGRDPAL_Shade_Color_3 0x01 << 7 + 0x01 << 6
#define MASK_IO_BGRDPAL_Shade_Color_2 0x01 << 5 + 0x01 << 4
#define MASK_IO_BGRDPAL_Shade_Color_1 0x01 << 3 + 0x01 << 2
#define MASK_IO_BGRDPAL_Shade_Color_0 0x01 << 1 + 0x01 << 0

#define OPT_Color_White      0
#define OPT_Color_Light_Gray 1
#define OPT_Color_Dark_Gray  2
#define OPT_Color_Black      3


static uint8_t frame_buffer[256 * 256];
