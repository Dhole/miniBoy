#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "debugger.h"
#include "memory.h"
#include "lr35902.h"
#include "linenoise/linenoise.h"

//#define MEM_SIZE 0x10000

typedef struct {
	char name[16];
	int arg_a, arg_b;
} command_t;

typedef enum {
	DBG_IDLE,
	DBG_CONT,
	DBG_STEP
} dbg_state_t;

const char *PROMPT = "miniBoy> ";
const int ARG_LEN = 16;

static uint8_t break_points[MEM_SIZE];
static regs_t *regs;
static dbg_state_t state;
static int rem_steps;
static command_t com;

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
	       "disas [start] [end]\n"
	       "io\n"
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

void com_disas(int start, int end) {
	if (start == -1) {
		start = regs->PC;
		end = start + 16;
	} else if (end == -1) {
		end = start + 16;
	}
	
	while (end > start) {
		start += disas_op(start);
	}
}

void com_step(int a) {
	if (a < 0) {
		a = 1;
	}
	rem_steps = a;
	state = DBG_STEP;
}

void debug_init() {
	int i;
	for (i = 0; i < MEM_SIZE; i++) {
		break_points[i] = 0;
	}
	state = DBG_IDLE;
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

	// TODO: Make this nicer: in a loop?
	
	if (strncmp(name, "help", ARG_LEN) == 0 || name[0] == 'h') {
		com_help();
	} else if (strncmp(name, "step", ARG_LEN) == 0 || name[0] == 's') {
		com_step(com->arg_a);
		return 1;
	} else if (strncmp(name, "run", ARG_LEN) == 0) {
		return -1;
	} else if (strncmp(name, "regs", ARG_LEN) == 0 || name[0] == 'r') {
		cpu_dump_reg();
	} else if (strncmp(name, "break", ARG_LEN) == 0 || name[0] == 'b') {
		com_break(com->arg_a);
	} else if (strncmp(name, "clear", ARG_LEN) == 0) {
		com_clear(com->arg_a);
	} else if (strncmp(name, "continue", ARG_LEN) == 0 || name[0] == 'c') {
		state = DBG_CONT;
		return 1;
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
		exit(0);
	} else {
		printf("E) Unrecognized command: %s\n", name);
	}
        return 0;
}

int debug_run(int *debug_flag, int *debug_pause) {
	char *line;
	int res;

	regs = cpu_get_regs();

	if (*debug_pause) {
		state = DBG_IDLE;
		*debug_pause = 0;
	}
	
	switch(state) {
	case DBG_IDLE:
		break;
	case DBG_STEP:
		if (rem_steps > 0) {
			disas_op(regs->PC);
			rem_steps--;
			return cpu_step();
		} else {
			state = DBG_IDLE;
		}
		break;
	case DBG_CONT:
		if (break_points[regs->PC]) {
			disas_op(regs->PC);
			state = DBG_IDLE;
			break;
		} else {
			/*
			if (mem_read_8(regs->PC) == 0xFB) {
				state = DBG_IDLE;
				break;
			}
			*/
			return cpu_step();
		}
		break;
	}

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
		res = run_com(&com);
		if (res < 0) {
			*debug_flag = 0;
			return 0;
		} else if (res == 0) {
			continue;
		} else {
			if (state == DBG_CONT) {
				return cpu_step();
			}
			return 0;
		}
	}
	return 0;
}
