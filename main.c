#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dmg.h"

void usage(char *argv0) {
	printf("Usage: %s [-v] [-b bios] rom\n", argv0);
	exit(1);
}

int main(int argc, char** argv) {
	int opt;
	int v_flag = 0;
	char *bios_path = NULL;
	char *rom_path = NULL;

	while ((opt = getopt(argc, argv, "vb:")) != -1) {
		switch (opt) {
		case 'b':
			bios_path = optarg;
			break;
		case 'v':
			v_flag = 1;
			break;
		default:
		        usage(argv[0]);
		}
	}
	if (optind == argc) {
		usage(argv[0]);
	}
	rom_path = argv[optind];

	load_bios(bios_path);
	load_rom(rom_path);
	mem_dump();

	return 0;
}
