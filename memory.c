#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"
#include "io_regs.h"


static uint8_t mm[MEM_SIZE];

uint8_t *mem_get_mem() {
	return mm;
}

void mem_bit_set(uint16_t addr, uint8_t bit) {
	mm[addr] |= bit;
}

void mem_bit_unset(uint16_t addr, uint8_t bit) {
	mm[addr] &= ~bit;
}

void mem_bit_test(uint16_t addr, uint8_t bit) {
	return ((mm[addr] & bit) > 0);
}

uint8_t mem_read_8(uint16_t addr) {
	return mm[addr];
	// Handle IO mappings???
}

uint16_t mem_read_16(uint16_t addr) {
	//return (uint16_t)mm[addr] + ((uint16_t)mm[addr + 1] << 8);
	return (uint16_t)mem_read_8(addr) +
		((uint16_t)mem_read_8(addr + 1) << 8);
}

void mem_write_8(uint16_t addr, uint8_t v) {
	mm[addr] = v;
	// Handle IO mappings
}

void mem_write_16(uint16_t addr, uint16_t v) {
	//mm[addr] = (uint8_t)(v & 0x00FF);
	//mm[addr + 1] = (uint8_t)((v & 0xFF00) >> 8);
	mem_write_8(addr, (uint8_t)(v & 0x00FF));
	mem_write_8(addr + 1, (uint8_t)((v & 0xFF00) >> 8));
}

void mem_load(uint16_t addr, uint8_t *buf, uint16_t size) {
	memcpy(&mm[addr], buf, size);
}

void mem_dump(uint16_t start, uint16_t end) {
	//unsigned char *buf = (unsigned char*)&memory;
	int i, j;
	for (i = start; i <= end; i += 16) {
		printf("%04X  ", i);
		for (j = 0; j < 16; j++) {
			if (i + j <= end) {
				printf("%02x ", mm[i+j]);
			} else {
				printf("   ");
			}
			if (j == 7) {
				printf(" ");
			}
		}
		printf(" |");
		for (j = 0; j < 16; j++) {
			if (i + j <= end) {
				printf("%c", isprint(mm[i+j]) ? mm[i+j] : '.');
			}
		}
		printf("|\n");
	}
}

char *byte2bin_str(uint8_t b, char *s) {
	int i;
	for (i = 0; i < 8; i++) {
		s[i] = ((b & (1 << i)) >> i) + 48;
	}
	s[i] = '\0';
	return s;
}

void mem_dump_io_regs() {
	char s[16];
printf("%02X [JOYPAD]:    %02x (%s)\n",
IO_JOYPAD, mm[IO_JOYPAD], byte2bin_str(mm[IO_JOYPAD], s));
printf("%02X [SIODATA]:   %02x (%s)\n",
IO_SIODATA, mm[IO_SIODATA], byte2bin_str(mm[IO_SIODATA], s));
printf("%02X [SIOCONT]:   %02x (%s)\n",
IO_SIOCONT, mm[IO_SIOCONT], byte2bin_str(mm[IO_SIOCONT], s));
printf("%02X [DIVIDER]:   %02x (%s)\n",
IO_DIVIDER, mm[IO_DIVIDER], byte2bin_str(mm[IO_DIVIDER], s));
printf("%02X [TIMECNT]:   %02x (%s)\n",
IO_TIMECNT, mm[IO_TIMECNT], byte2bin_str(mm[IO_TIMECNT], s));
printf("%02X [TIMEMOD]:   %02x (%s)\n",
IO_TIMEMOD, mm[IO_TIMEMOD], byte2bin_str(mm[IO_TIMEMOD], s));
printf("%02X [TIMCONT]:   %02x (%s)\n",
IO_TIMCONT, mm[IO_TIMCONT], byte2bin_str(mm[IO_TIMCONT], s));
printf("%02X [IFLAGS]:    %02x (%s)\n",
IO_IFLAGS, mm[IO_IFLAGS], byte2bin_str(mm[IO_IFLAGS], s));
printf("%02X [SNDREG10]:  %02x (%s)\n",
IO_SNDREG10, mm[IO_SNDREG10], byte2bin_str(mm[IO_SNDREG10], s));
printf("%02X [SNDREG11]:  %02x (%s)\n",
IO_SNDREG11, mm[IO_SNDREG11], byte2bin_str(mm[IO_SNDREG11], s));
printf("%02X [SNDREG12]:  %02x (%s)\n",
IO_SNDREG12, mm[IO_SNDREG12], byte2bin_str(mm[IO_SNDREG12], s));
printf("%02X [SNDREG13]:  %02x (%s)\n",
IO_SNDREG13, mm[IO_SNDREG13], byte2bin_str(mm[IO_SNDREG13], s));
printf("%02X [SNDREG14]:  %02x (%s)\n",
IO_SNDREG14, mm[IO_SNDREG14], byte2bin_str(mm[IO_SNDREG14], s));
printf("%02X [SNDREG21]:  %02x (%s)\n",
IO_SNDREG21, mm[IO_SNDREG21], byte2bin_str(mm[IO_SNDREG21], s));
printf("%02X [SNDREG22]:  %02x (%s)\n",
IO_SNDREG22, mm[IO_SNDREG22], byte2bin_str(mm[IO_SNDREG22], s));
printf("%02X [SNDREG23]:  %02x (%s)\n",
IO_SNDREG23, mm[IO_SNDREG23], byte2bin_str(mm[IO_SNDREG23], s));
printf("%02X [SNDREG24]:  %02x (%s)\n",
IO_SNDREG24, mm[IO_SNDREG24], byte2bin_str(mm[IO_SNDREG24], s));
printf("%02X [SNDREG30]:  %02x (%s)\n",
IO_SNDREG30, mm[IO_SNDREG30], byte2bin_str(mm[IO_SNDREG30], s));
printf("%02X [SNDREG31]:  %02x (%s)\n",
IO_SNDREG31, mm[IO_SNDREG31], byte2bin_str(mm[IO_SNDREG31], s));
printf("%02X [SNDREG32]:  %02x (%s)\n",
IO_SNDREG32, mm[IO_SNDREG32], byte2bin_str(mm[IO_SNDREG32], s));
printf("%02X [SNDREG33]:  %02x (%s)\n",
IO_SNDREG33, mm[IO_SNDREG33], byte2bin_str(mm[IO_SNDREG33], s));
printf("%02X [SNDREG34]:  %02x (%s)\n",
IO_SNDREG34, mm[IO_SNDREG34], byte2bin_str(mm[IO_SNDREG34], s));
printf("%02X [SNDREG41]:  %02x (%s)\n",
IO_SNDREG41, mm[IO_SNDREG41], byte2bin_str(mm[IO_SNDREG41], s));
printf("%02X [SNDREG42]:  %02x (%s)\n",
IO_SNDREG42, mm[IO_SNDREG42], byte2bin_str(mm[IO_SNDREG42], s));
printf("%02X [SNDREG43]:  %02x (%s)\n",
IO_SNDREG43, mm[IO_SNDREG43], byte2bin_str(mm[IO_SNDREG43], s));
printf("%02X [SNDREG44]:  %02x (%s)\n",
IO_SNDREG44, mm[IO_SNDREG44], byte2bin_str(mm[IO_SNDREG44], s));
printf("%02X [SNDREG50]:  %02x (%s)\n",
IO_SNDREG50, mm[IO_SNDREG50], byte2bin_str(mm[IO_SNDREG50], s));
printf("%02X [SNDREG51]:  %02x (%s)\n",
IO_SNDREG51, mm[IO_SNDREG51], byte2bin_str(mm[IO_SNDREG51], s));
printf("%02X [SNDREG52]:  %02x (%s)\n",
IO_SNDREG52, mm[IO_SNDREG52], byte2bin_str(mm[IO_SNDREG52], s));
printf("%02X [LCDCONT]:   %02x (%s)\n",
IO_LCDCONT, mm[IO_LCDCONT], byte2bin_str(mm[IO_LCDCONT], s));
printf("%02X [LCDSTAT]:   %02x (%s)\n",
IO_LCDSTAT, mm[IO_LCDSTAT], byte2bin_str(mm[IO_LCDSTAT], s));
printf("%02X [SCROLLY]:   %02x (%s)\n",
IO_SCROLLY, mm[IO_SCROLLY], byte2bin_str(mm[IO_SCROLLY], s));
printf("%02X [SCROLLX]:   %02x (%s)\n",
IO_SCROLLX, mm[IO_SCROLLX], byte2bin_str(mm[IO_SCROLLX], s));
printf("%02X [CURLINE]:   %02x (%s)\n",
IO_CURLINE, mm[IO_CURLINE], byte2bin_str(mm[IO_CURLINE], s));
printf("%02X [CMPLINE]:   %02x (%s)\n",
IO_CMPLINE, mm[IO_CMPLINE], byte2bin_str(mm[IO_CMPLINE], s));
printf("%02X [BGRDPAL]:   %02x (%s)\n",
IO_BGRDPAL, mm[IO_BGRDPAL], byte2bin_str(mm[IO_BGRDPAL], s));
printf("%02X [OBJ0PAL]:   %02x (%s)\n",
IO_OBJ0PAL, mm[IO_OBJ0PAL], byte2bin_str(mm[IO_OBJ0PAL], s));
printf("%02X [OBJ1PAL]:   %02x (%s)\n",
IO_OBJ1PAL, mm[IO_OBJ1PAL], byte2bin_str(mm[IO_OBJ1PAL], s));
printf("%02X [WNDPOSY]:   %02x (%s)\n",
IO_WNDPOSY, mm[IO_WNDPOSY], byte2bin_str(mm[IO_WNDPOSY], s));
printf("%02X [WNDPOSX]:   %02x (%s)\n",
IO_WNDPOSX, mm[IO_WNDPOSX], byte2bin_str(mm[IO_WNDPOSX], s));
printf("%02X [DMACONT]:   %02x (%s)\n",
IO_DMACONT, mm[IO_DMACONT], byte2bin_str(mm[IO_DMACONT], s));
printf("%02X [IENABLE]:   %02x (%s)\n",
       IO_IENABLE, mm[IO_IENABLE], byte2bin_str(mm[IO_IENABLE], s));
}
