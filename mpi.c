#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "tiles.h"


void writeBoardToFile(struct Board_square *b, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open output file");
        return;
    }
    for (int i = 0; i < b->height; i++) {
        for (int j = 0; j < b->width; j++) {
            fprintf(fp, "%d ", b->tiles[i][j].value);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

}

// Parallel Abelian sandpile using 1D row decomposition with timing
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int width, height;
    if (rank == 0) {
        printf("Enter board width: "); fflush(stdout);
        scanf("%d", &width);
        printf("Enter board height: "); fflush(stdout);
        scanf("%d", &height);
    }
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Start timer after input distribution
    double t_start = MPI_Wtime();

    // Determine rows per rank
    int base = height / size;
    int rem = height % size;
    int local_rows = base + (rank < rem ? 1 : 0);
    int H = local_rows;
    int W = width;
    int total = H + 2; // includes halos

    // Allocate tiles with two halo rows
    int **tiles = malloc(total * sizeof(int*));
    for (int i = 0; i < total; i++) {
        tiles[i] = calloc(W, sizeof(int));
    }

    // Initialize interior cells
    for (int i = 1; i <= H; i++) {
        for (int j = 0; j < W; j++) {
            tiles[i][j] = 4;
        }
    }

    // Stabilize board
    int global_changed = 1;
    while (global_changed) {
        int local_changed = 0;
        int *send_up   = calloc(W, sizeof(int));
        int *send_down = calloc(W, sizeof(int));

        // Toppling
        for (int i = 1; i <= H; i++) {
            for (int j = 0; j < W; j++) {
                int v = tiles[i][j];
                if (v >= 4) {
                    int d = v / 4;
                    if (j > 0) tiles[i][j-1] += d;
                    if (j < W-1) tiles[i][j+1] += d;
                    if (i > 1) tiles[i-1][j] += d;
                    else send_up[j] += d;
                    if (i < H) tiles[i+1][j] += d;
                    else send_down[j] += d;
                    tiles[i][j] = v % 4;
                    local_changed = 1;
                }
            }
        }

        // Exchange halos
        MPI_Request reqs[4]; int rc = 0;
        if (rank > 0) {
            MPI_Isend(send_up, W, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &reqs[rc++]);
            MPI_Irecv(tiles[0], W, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &reqs[rc++]);
        }
        if (rank < size-1) {
            MPI_Isend(send_down, W, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &reqs[rc++]);
            MPI_Irecv(tiles[H+1], W, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &reqs[rc++]);
        }
        MPI_Waitall(rc, reqs, MPI_STATUSES_IGNORE);
        free(send_up);
        free(send_down);

        // Apply halos
        for (int j = 0; j < W; j++) {
            if (tiles[0][j] > 0) {
                tiles[1][j] += tiles[0][j];
                local_changed = 1;
                tiles[0][j] = 0;
            }
            if (tiles[H+1][j] > 0) {
                tiles[H][j] += tiles[H+1][j];
                local_changed = 1;
                tiles[H+1][j] = 0;
            }
        }

        // Check for stabilization
        MPI_Allreduce(&local_changed, &global_changed, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
    }

    // Gather stabilized state to root
    int *outbuf = malloc(H * W * sizeof(int));
    for (int i = 0; i < H; ++i)
        for (int j = 0; j < W; ++j)
            outbuf[i*W + j] = tiles[i+1][j];

    int *counts = NULL, *displs = NULL;
    int *global = NULL;
    if (rank == 0) {
        counts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        global = malloc(height * W * sizeof(int));
        int offset = 0;
        for (int r = 0; r < size; r++) {
            int rows = base + (r < rem ? 1 : 0);
            counts[r] = rows * W;
            displs[r] = offset * W;
            offset += rows;
        }
    }

    MPI_Gatherv(outbuf, H*W, MPI_INT,
                global, counts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    // End timer
    double t_end = MPI_Wtime();

    if (rank == 0) {
        struct Board_square board;
        board.height = height;
        board.width  = W;

        // allocate the 2D tiles array in board
        board.tiles = malloc(board.height * sizeof(*board.tiles));
        for (int i = 0; i < board.height; i++) {
            board.tiles[i] = calloc(board.width, sizeof(*board.tiles[i]));
            for (int j = 0; j < board.width; j++) {
                board.tiles[i][j].value = global[i * board.width + j];
            }
        }

        // write out once to “board.txt”
        writeBoardToFile(&board, "board.txt");
        // write again to “mpi.out”
        writeBoardToFile(&board, "mpi.out");

        // clean up
        for (int i = 0; i < board.height; i++) {
            free(board.tiles[i]);
        }
        free(board.tiles);

        printf("Board written to board.txt and mpi.out\n");
        printf("Total execution time: %.6f seconds\n", t_end - t_start);

        free(global);
        free(counts);
        free(displs);
    }


    // Cleanup
    free(outbuf);
    for (int i = 0; i < total; i++) free(tiles[i]);
    free(tiles);

    MPI_Finalize();
    return 0;
}
