DEBUG ?= 1
CFLAGS = -Wall -Werror
SDL = -lSDL2

ifeq ($(DEBUG), 1)
    DBGFLAGS = -DDEBUG -g
else
    DBGFLAGS = -DNDEBUG
endif

CC = clang $(CFLAGS) $(DBGFLAGS)

#all: miniBoy lr35902_test
all: miniBoy

#lr35902_test: lr35902.o memory.o string_fun.o lr35902_test.c
#	clang lr35902.o memory.o string_fun.o lr35902_test.c -o lr35902_test

miniBoy: sdl.o screen.o timer.o string_fun.o insertion_sort.o linenoise.o debugger.o lr35902.o dmg.o rom.o memory.o main.c
	$(CC) sdl.o screen.o timer.o string_fun.o insertion_sort.o linenoise.o debugger.o lr35902.o dmg.o rom.o memory.o main.c -o miniBoy $(SDL)

lr35902.o: lr35902.c
	$(CC) -c lr35902.c

dmg.o: dmg.c
	$(CC) -c dmg.c

memory.o: memory.c
	$(CC) -c memory.c

rom.o: rom.c
	$(CC) -c rom.c

debugger.o: debugger.c
	$(CC) -c debugger.c

linenoise.o: linenoise/linenoise.c
	$(CC) -c linenoise/linenoise.c

string_fun.o: string_fun.c
	$(CC) -c string_fun.c

insertion_sort.o: insertion_sort.c
	$(CC) -c insertion_sort.c

screen.o: screen.c
	$(CC) -c screen.c

timer.o: timer.c
	$(CC) -c timer.c

sdl.o: sdl.c
	$(CC) -c sdl.c

clean:
	rm -rf *.o miniBoy
