# Compiler variables
CC = gcc
MPICC = mpicc

# Flags
CFLAGS = -Wall -O2 -std=c99
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

check: asyncserial openmp mpi
	@echo "=== Running serial async ==="
	@printf "513 513\n" | ./asyncserial
	@echo "=== Running OpenMP ==="
	@printf "513 513\n" | ./openmp
	@echo "=== Running MPI (4 ranks) ==="
	@printf "513 513\n" | mpirun -np 4 ./mpi

	@echo "=== Diff serial vs OpenMP ==="
	@diff serial.out openmp.out \
		&& echo "OpenMP matches serial" \
		|| (echo "OpenMP mismatch!" && exit 1)

	@echo "=== Diff serial vs MPI ==="
	@diff serial.out mpi.out \
		&& echo "MPI matches serial" \
		|| (echo "MPI mismatch!" && exit 1)

