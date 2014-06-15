DEBUG ?= 1

ifeq ($(DEBUG), 1)
    CFLAGS = -DDEBUG -g
else
    CFLAGS = -DNDEBUG
endif

CC = gcc $(CFLAGS)

all: miniBoy

miniBoy: lr35902.o dmg.o memory.o main.c
	$(CC) lr35902.o dmg.o memory.o main.c -o miniBoy

lr35902.o: lr35902.c
	$(CC) -c lr35902.c

dmg.o: dmg.c
	$(CC) -c dmg.c

memory.o: memory.c
	$(CC) -c memory.c

clean:
	rm -rf *.o miniBoy
