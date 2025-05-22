all: main.c
	gcc -o Sandpiles main.c
	gcc -o SandpileOpenmp AbelianSandpileOpenmp.c -fopenmp

openmp:
	gcc -o SandpileOpenmp AbelianSandpileOpenmp.c -fopenmp

clean:
	$(RM)  Sandpiles SandpileOpenmp board.txt *.o *.exe output.png

run:
	./Sandpiles