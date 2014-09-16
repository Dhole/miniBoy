#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "memory.h"
#include "lr35902.h"
#include "linenoise/linenoise.h"

//#define MEM_SIZE 0x10000

const char *PROMPT = "miniBoy> ";
const int ARG_LEN = 16;

uint8_t break_points[MEM_SIZE];
regs_t *regs;

typedef struct {
	char name[16];
	int arg_a, arg_b;
} command_t;

void com_help() {
	printf("\n    -= miniBoy debugger commands: =-\n\n"
	       "help\n"
	       "step [n]\n"
	       "regs\n"
	       "run\n"
	       "break n\n"
	       "continue\n"
	       "clear [n]\n"
	       "memory [start] [end]\n"
	       "quit\n\n");
}

void com_clear(int n) {
	int i;
	if (n == -1) {
		printf("- Clearing all breakpoints\n");
		for (i = 0; i < MEM_SIZE; i++) {
			break_points[i] = 0;
		}
	} else {
		printf("- Clearing breakpoint 0x%02X\n", n);
		break_points[n] = 0;
	}
}

void com_break(int n) {
	if (n < 0) {
		printf("E) Breakpoint must be positive\n");
		return;
	} else {
		printf("- Setting breakpoint at 0x%02X\n", n);
		break_points[n] = 1;
	}
}

void com_disas(int a, int b) {
	int next = a;

	while (b > next) {
		next += disas_op(next);
	}
}

void com_step(int a) {
	int i;
	
	if (a < 0) {
		a = 1;
	}
	for (i = 0; i < a; i++) {
		disas_op(regs->PC);
		cpu_step();
	}
}

void init_debug() {
        com_clear(-1);
	regs = cpu_get_regs();
}

int parse_com(char *buf, command_t *com, int arg_len) {
	char *pch, *endptr;
	int *args[2] = {&com->arg_a, &com->arg_b};
	int i;
	
	com->name[0] = '\0';
	com->arg_a = -1;
	com->arg_b = -1;

	pch = strtok(buf, " ");
	if (pch != NULL) {
		strncpy(com->name, pch, arg_len);
		com->name[arg_len - 1] = '\0';
	} else {
		return -1;
	}
	for (i = 0; i < 2; i++) {
		pch = strtok(NULL, " ");
		if (pch != NULL) {
			if (pch[1] == 'x') {
				*args[i] = strtol(pch, &endptr, 16);
			} else {
				*args[i] = strtol(pch, &endptr, 10);
			}
			if (endptr == pch) {
				*args[i] = -2;
				return -2;
			}
		} else {
			return i;
		}
	}
	return 2;	
}

int run_com(command_t *com) {
	char *name = com->name;
	
	if (strncmp(name, "help", ARG_LEN) == 0 || name[0] == 'h') {
		com_help();
	} else if (strncmp(name, "step", ARG_LEN) == 0 || name[0] == 's') {
		com_step(com->arg_a);
	} else if (strncmp(name, "run", ARG_LEN) == 0) {
		printf("run!\n");
	} else if (strncmp(name, "regs", ARG_LEN) == 0 || name[0] == 'r') {
		cpu_dump_reg();
	} else if (strncmp(name, "break", ARG_LEN) == 0 || name[0] == 'b') {
		com_break(com->arg_a);
	} else if (strncmp(name, "clear", ARG_LEN) == 0) {
		com_clear(com->arg_a);
	} else if (strncmp(name, "continue", ARG_LEN) == 0 || name[0] == 'c') {
		printf("continue!\n");
	} else if (strncmp(name, "memory", ARG_LEN) == 0 || name[0] == 'm') {
		//printf("memory!\n");
		mem_dump(com->arg_a, com->arg_b);
	} else if (strncmp(name, "disas", ARG_LEN) == 0 || name[0] == 'd') {
		//printf("memory!\n");
		com_disas(com->arg_a, com->arg_b);
	} else if (strncmp(name, "io", ARG_LEN) == 0 || name[0] == 'i') {
		//printf("memory!\n");
		mem_dump_io_regs();
	} else if (strncmp(name, "quit", ARG_LEN) == 0 || name[0] == 'q') {
		return -2;
	} else {
		printf("E) Unrecognized command: %s\n", name);
		return -1;
	}
	return 0;
}

int debug_run() {
	char *line;
	command_t com;

        init_debug();

	while((line = linenoise(PROMPT)) != NULL) {
		if (line[0] != '\0') {
			linenoiseHistoryAdd(line);
			if (parse_com(line, &com, ARG_LEN) < 0) {
				printf("E) Error parsing command: %s\n", line);
				free(line);
				continue;
			}
		}		
		free(line);
		if (run_com(&com) == -2) {
			break;
		}
	}
	return 0;
}
