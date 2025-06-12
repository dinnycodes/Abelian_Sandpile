
# Abelian Sandpile HPC Assignment 

## Overview

This code implements the Abelian Sandpile simulation using:

* Serial asynchronous version (`asyncserial.c`) - this is our baseline to compare the parallel versions with
* Serial synchronous version (`syncserial.c`)
* OpenMP parallel version (`openmp.c`)
* MPI parallel version (`mpi.c`)

The goal of this project is to compare the performance and scaling of different parallelisation approaches (OpenMP and MPI) using the CHPC Lengau cluster.

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

## Visualising the Board

You can generate an image of the board output using the provided `Render_board.py` script.

Make sure you have a valid `board.txt` file (produced by the Serial, OpenMP or MPI versions).

Then run:

```bash
python3 Render_board.py
```

This will read `board.txt` and generate a PNG image (`output.png`) showing the final stabilised board.


---

## How to verify correctness

We provide a `make check` command that will automatically run the serial and parallel versions with a 513×513 grid, and compare their outputs.
Note: You will be prompted for input grid size but do not provide it because it is already set to 513x513 in the Makefile.

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
## Running OpenMP on CHPC

We provide the following PBS scripts to run OpenMP jobs:

* `omp_problem.pbs` — tests for different problem sizes from 129 to 1025 with fixed number of threads (20)
* `omp_strongscaling.pbs` — strong scaling (fixed problem size to 513x513, varying threads from 2 to 24)
* `omp_weakscaling.pbs` — weak scaling (problem size grows with threads)


### Submit job:

   ```
   qsub omp_problem.pbs
   qsub omp_strongscaling.pbs
   qsub omp_weakscaling.pbs
   ```

Outputs are written to the `omp_problemscaling_output`, `omp_strongscaling_output`, or `omp_weakscaling_output` folders.



---
## Running MPI on the CHPC cluster
---
## Dependencies

* GCC with OpenMP support
* MPI compiler (e.g. `mpicc`)
* Intel compiler with MPI

### Submit job:

   ```
   qsub Intel_2018_10node24_test.pbs

Outputs are written to the same folder as the batch script.







  
