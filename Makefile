all: main.c
	gcc -o Sandpiles main.c

clean:
	$(RM)  Sandpiles board.txt *.o *.exe output.png

run:
	./Sandpiles