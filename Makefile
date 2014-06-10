all: miniBoy

miniBoy: lr35902.o dmg.o memory.o main.c
	gcc lr35902.o dmg.o memory.o main.c -o miniBoy

lr35902.o: lr35902.c
	gcc -c lr35902.c

dmg.o: dmg.c
	gcc -c dmg.c

memory.o: memory.c
	gcc -c memory.c

clean:
	rm -rf *.o miniBoy
