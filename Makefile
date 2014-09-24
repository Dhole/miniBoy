DEBUG ?= 1
CFLAGS = -Wall -Werror

ifeq ($(DEBUG), 1)
    DBGFLAGS = -DDEBUG -g
else
    DBGFLAGS = -DNDEBUG
endif

CC = gcc $(CFLAGS) $(DBGFLAGS)

all: miniBoy

miniBoy: string_fun.o linenoise.o debugger.o lr35902.o dmg.o memory.o main.c
	$(CC) string_fun.o linenoise.o debugger.o lr35902.o dmg.o memory.o main.c -o miniBoy

lr35902.o: lr35902.c
	$(CC) -c lr35902.c

dmg.o: dmg.c
	$(CC) -c dmg.c

memory.o: memory.c
	$(CC) -c memory.c

debugger.o: debugger.c
	$(CC) -c debugger.c

linenoise.o: linenoise/linenoise.c
	$(CC) -c linenoise/linenoise.c

string_fun.o: string_fun.c
	$(CC) -c string_fun.c

clean:
	rm -rf *.o miniBoy
