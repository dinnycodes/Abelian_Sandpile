CC = gcc
CFLAGS = -Wall -O2
OMPFLAGS = -fopenmp

# Default target: build all three versions
all: sandpile openmp run_asyncserial asyncopenmp 

# Serial version build
sandpile: main.c
	$(CC) $(CFLAGS) -o Sandpiles main.c

# OpenMP version build
openmp: AbelianSandpileOpenmp.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o SandpileOpenmp AbelianSandpileOpenmp.c

# Async OpenMP version build
asyncopenmp: AsyncOpenmp.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o AsyncOpenmp AsyncOpenmp.c

# Async Serial version build
asyncserial: AsyncSerial.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o AsyncSerial AsyncSerial.c

# Run all versions
run_all: run_serial run_openmp run_asyncserial  run_async_openmp

run_serial: sandpile
	./Sandpiles

run_openmp: openmp
	./SandpileOpenmp

run_asyncserial: asyncserial
	./AsyncSerial

run_asyncopenmp: asyncopenmp
	./AsyncOpenmp

# Clean up all generated files
clean:
	$(RM) Sandpiles SandpileOpenmp AsyncOpenmp AsyncSerial board.txt *.o *.exe output.png
