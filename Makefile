all: Sandpiles

Sandpiles: main.c
	gcc -o Sandpiles main.c

run: Sandpiles
	./Sandpiles

clean:
	$(RM) Sandpiles
