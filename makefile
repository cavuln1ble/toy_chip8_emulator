main: main.o
	gcc -o main main.o
	make clear

main.o: main.c chip8.c chip8.h
	gcc -c main.c chip8.c

clear: 
	del *.o

run:
	make main
	./main
