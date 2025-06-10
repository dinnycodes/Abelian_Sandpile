# Compiler variables
CC = gcc
MPICC = mpicc

# Flags
CFLAGS = -Wall -O2
OMPFLAGS = -fopenmp

# Targets
all: asyncserial syncserial openmp mpi

# Serial async version
asyncserial: asyncserial.c
	$(CC) $(CFLAGS) -o asyncserial asyncserial.c

# Serial sync version
syncserial: syncserial.c
	$(CC) $(CFLAGS) -o syncserial syncserial.c

# OpenMP version
openmp: openmp.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o openmp openmp.c

# MPI version
mpi: mpi.c
	$(MPICC) $(CFLAGS) -o mpi mpi.c

# Run targets
run_asyncserial: asyncserial
	./asyncserial

run_syncserial: syncserial
	./syncserial

run_openmp: openmp
	./openmp

run_mpi: mpi
	mpirun -np 7 ./mpi

# Clean target
clean:
	$(RM) asyncserial syncserial openmp mpi *.txt *.png *.out

check: asyncserial mpi
	@echo "=== Running serial async ==="
	@printf "129 129\n" | ./asyncserial
	@echo "=== Running MPI (4 ranks) ==="
	@printf "129 129\n" | mpirun -np 4 ./mpi
	@echo "=== Diffing outputs ==="
	@diff serial.out mpi.out && echo "Match! Correctness verified" || (echo "Mismatch! Parallel is not corect." && exit 1)