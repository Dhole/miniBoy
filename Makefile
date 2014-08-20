DEBUG ?= 1

ifeq ($(DEBUG), 1)
    CFLAGS = -DDEBUG -g
else
    CFLAGS = -DNDEBUG
endif

CC = gcc $(CFLAGS)

all: miniBoy

miniBoy: string_fun.c lr35902.o dmg.o memory.o main.c
	$(CC) string_fun.o lr35902.o dmg.o memory.o main.c -o miniBoy

lr35902.o: lr35902.c
	$(CC) -c lr35902.c

dmg.o: dmg.c
	$(CC) -c dmg.c

memory.o: memory.c
	$(CC) -c memory.c

string_fun.o: string_fun.c
	$(CC) -c string_fun.c

clean:
	rm -rf *.o miniBoy
