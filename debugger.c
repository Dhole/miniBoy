#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linenoise/linenoise.h"

const char *PROMPT = "miniBoy> ";
const int ARG_LEN = 16;

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
	       "break\n"
	       "continue\n"
	       "clear [n]\n"
	       "memory [start] [end]\n"
	       "quit\n\n");
}

void completion(const char *buf, linenoiseCompletions *lc) {
	switch (buf[0]) {
	case 'h':
		linenoiseAddCompletion(lc, "help ");
		break;
	case 's':
		linenoiseAddCompletion(lc, "step ");
		break;
	case 'r':
		linenoiseAddCompletion(lc, "regs ");
		linenoiseAddCompletion(lc, "run ");
		break;
	case 'b':
		linenoiseAddCompletion(lc, "break ");
		break;
	case 'c':
		linenoiseAddCompletion(lc, "continue ");
		linenoiseAddCompletion(lc, "clear ");
		break;
	case 'm':
		linenoiseAddCompletion(lc, "memory ");
		break;
	case 'q':
		linenoiseAddCompletion(lc, "quit ");
		break;
	}
}

int parse_com(char *buf, command_t *com, int arg_len) {
	char *pch, *endptr;
       
	com->name[0] = '\0';
	com->arg_a = 0;
	com->arg_b = 0;

	pch = strtok(buf, " ");
	if (pch != NULL) {
		strncpy(com->name, pch, arg_len);
		com->name[arg_len - 1] = '\0';
	} else {
		return -1;
	}
	pch = strtok(NULL, " ");
	if (pch != NULL) {
		if (pch[1] == 'x') {
			com->arg_a = strtol(pch, &endptr, 16);
		} else {
			com->arg_a = strtol(pch, &endptr, 10);
		}
		if (endptr == pch) {
			com->arg_a = -1;
			return -2;
		}
	} else {
		return 1;
	}
	pch = strtok(NULL, " ");
	if (pch != NULL) {
		if (pch[1] == 'x') {
			com->arg_b = strtol(pch, &endptr, 16);
		} else {
			com->arg_b = strtol(pch, &endptr, 10);
		}
		if (endptr == pch) {
			com->arg_b = -1;
			return -2;
		}
	} else {
		return 2;
	}
	return 3;
	
}

int run_com(command_t *com) {
	char *name = com->name;
	
	if (strncmp(name, "help", ARG_LEN) == 0 || name[0] == 'h') {
		com_help();
	} else if (strncmp(name, "step", ARG_LEN) == 0 || name[0] == 's') {
		printf("step!\n");
	} else if (strncmp(name, "regs", ARG_LEN) == 0 || name[0] == 'r') {
		printf("regs!\n");
	} else if (strncmp(name, "run", ARG_LEN) == 0) {
		printf("run!\n");
	} else if (strncmp(name, "break", ARG_LEN) == 0 || name[0] == 'b') {
		printf("break!\n");
	} else if (strncmp(name, "continue", ARG_LEN) == 0 || name[0] == 'c') {
		printf("continue!\n");
	} else if (strncmp(name, "clear", ARG_LEN) == 0) {
		printf("clear!\n");
	} else if (strncmp(name, "memory", ARG_LEN) == 0 || name[0] == 'm') {
		printf("memory!\n");
	} else if (strncmp(name, "quit", ARG_LEN) == 0 || name[0] == 'q') {
		return -2;
	} else {
		printf("Unrecognized command: %s\n", name);
		return -1;
	}
	return 0;
}

int main(int argc, char **argv) {
	char *line;
	command_t com;

	linenoiseSetCompletionCallback(completion);

	while((line = linenoise(PROMPT)) != NULL) {
		if (line[0] != '\0') {
			linenoiseHistoryAdd(line);
			if (parse_com(line, &com, ARG_LEN) < 0) {
				printf("Error parsing command: %s\n", line);
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
