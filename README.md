
# Abelian Sandpile HPC Assignment 

## Overview

This project implements the Abelian Sandpile simulation using:

* Serial asynchronous version (`asyncserial`)
* Serial synchronous version (`syncserial`)
* OpenMP parallel version (`openmp`)
* MPI parallel version (`mpi`)

The goal of this project was to compare the performance and scaling of different parallelisation approaches using the CHPC Lengau cluster.

---

## How to build the code

Run:

```bash
make
```

This will compile all four versions of the code:

* `asyncserial`
* `syncserial`
* `openmp`
* `mpi`

---

## How to run the code

You can run each version manually:

```bash
make run_asyncserial
make run_syncserial
make run_openmp
make run_mpi
```

Alternatively, you can run the binaries directly:

```bash
./asyncserial
./syncserial
./openmp
mpirun -np <number_of_processes> ./mpi
```

For all versions, you will be prompted to input:

```
<grid width>
<grid height>
```

---

## How to verify correctness

We provide a `make check` command that will automatically run the serial and parallel versions with a 513×513 grid, and compare their outputs.

```bash
make check
```

It will run:

* Serial async version
* OpenMP version
* MPI version (4 ranks)

Then it will diff the board outputs to check if they match:

```bash
=== Diff serial vs OpenMP ===
=== Diff serial vs MPI ===
```

A success message will be shown if the outputs match.

---

## Notes

* The `check` target is set to use a 513×513 grid. You can modify this in the `check` target if you want to test other grid sizes.
* MPI version is run with `mpirun -np 4` in the check, but you can adjust this as needed.
* Intermediate files (board outputs and images) can be cleaned using:

```bash
make clean
```

---

## Dependencies

* GCC with OpenMP support
* OpenMP
* MPI compiler (e.g. `mpicc`)
