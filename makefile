CFLAGS=-g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
LDFLAGS=-g -I include -L lib -lraylib -lgdi32 -lwinmm

chip8: $(OBJS)
	gcc $(CFLAGS) -o chip8 $(OBJS) $(LDFLAGS)

main.o: chip8.h
	gcc -g -c -o main.o main.c

chip8.o: chip8.h
	gcc -g -c -o chip8.o chip8.c

clean: 
	del chip8.exe *.o 

.PHONY: test clean