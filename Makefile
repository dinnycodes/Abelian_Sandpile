CC = gcc
CFLAGS = -Wall -O2
OMPFLAGS = -fopenmp

# Default target: build both versions
all: sandpile openmp

# Serial version build
sandpile: main.c
	$(CC) $(CFLAGS) -o Sandpiles main.c

# OpenMP version build
openmp: AbelianSandpileOpenmp.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o SandpileOpenmp AbelianSandpileOpenmp.c

# Run both
run_all: run_serial run_openmp

# Run serial
run_serial: sandpile
	./Sandpiles

# Run OpenMP
run_openmp: openmp
	./SandpileOpenmp

# Clean up all generated files
clean:
	$(RM) Sandpiles SandpileOpenmp board.txt *.o *.exe output.png
