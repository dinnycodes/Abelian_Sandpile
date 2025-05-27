#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include "tiles.h"

#define TAG_UPDATE 1
#define TAG_REQUEST 2
#define TAG_RESPONSE 3

// Print local portion of the board including halos (for debugging)
void printLocalBoardWithHalos(struct Board_square *b, int rank, int iter) {
    printf("Process %d local board with halos at iteration %d:\n", rank, iter);
    for (int i = 0; i < b->height; i++) {
        printf("[");
        for (int j = 0; j < b->width; j++) {
            printf(" %d ", b->tiles[i][j].value);
            if (j < b->width - 1) printf(",");
        }
        printf("]\n");
    }
    printf("\n");
}

// Write the full board to file (only rank 0 does this)
void writeBoardToFile(struct Board_square *b, const char *filename, int rank, int size) {
    if (rank == 0) {
        FILE *fp = fopen(filename, "w");
        if (!fp) {
            perror("Failed to open output file");
            return;
        }

        // Write rank 0's portion first
        for (int i = 1; i < b->height - 1; i++) { // exclude halos when writing to file
            for (int j = 0; j < b->width; j++) {
                fprintf(fp, "%d ", b->tiles[i][j].value);
            }
            fprintf(fp, "\n");
        }

        // Collect and write other processes' portions (exclude halos)
        for (int p = 1; p < size; p++) {
            int remote_height, remote_width;
            MPI_Recv(&remote_height, 1, MPI_INT, p, TAG_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&remote_width, 1, MPI_INT, p, TAG_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int *buffer = malloc(remote_height * remote_width * sizeof(int));
            MPI_Recv(buffer, remote_height * remote_width, MPI_INT, p, TAG_RESPONSE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < remote_height; i++) {
                for (int j = 0; j < remote_width; j++) {
                    fprintf(fp, "%d ", buffer[i * remote_width + j]);
                }
                fprintf(fp, "\n");
            }
            free(buffer);
        }
        fclose(fp);
    } else {
        // Send dimensions first (exclude halos)
        int send_height = b->height - 2;
        int send_width = b->width;
        MPI_Send(&send_height, 1, MPI_INT, 0, TAG_REQUEST, MPI_COMM_WORLD);
        MPI_Send(&send_width, 1, MPI_INT, 0, TAG_REQUEST, MPI_COMM_WORLD);

        // Flatten and send data excluding halos
        int *buffer = malloc(send_height * send_width * sizeof(int));
        for (int i = 1; i < b->height - 1; i++) {
            for (int j = 0; j < b->width; j++) {
                buffer[(i-1) * b->width + j] = b->tiles[i][j].value;
            }
        }
        MPI_Send(buffer, send_height * send_width, MPI_INT, 0, TAG_RESPONSE, MPI_COMM_WORLD);
        free(buffer);
    }
}

// Asynchronous toppling (only topple inner cells, not halos)
int async_topple(struct Board_square *b, int y, int x) {
    if (b->tiles[y][x].value < 4) return 0;

    int div4 = b->tiles[y][x].value / 4;

    if (x > 0)           b->tiles[y][x - 1].value += div4;
    if (x < b->width-1)  b->tiles[y][x + 1].value += div4;
    if (y > 0)           b->tiles[y - 1][x].value += div4;
    if (y < b->height-1) b->tiles[y + 1][x].value += div4;

    b->tiles[y][x].value %= 4;
    return 1;
}

// Exchange boundary rows with neighboring processes (only exchange inner edges, i.e., halos)
void exchangeBoundaries(struct Board_square *b, int rank, int size) {
    MPI_Request requests[4];
    int request_count = 0;

    // Send top row (row 1) to rank-1's bottom halo (row height-1)
    // Receive from rank-1 into top halo (row 0)
    if (rank > 0) {
        MPI_Isend(b->tiles[1], b->width, MPI_INT, rank-1, TAG_UPDATE, MPI_COMM_WORLD, &requests[request_count++]);
        MPI_Irecv(b->tiles[0], b->width, MPI_INT, rank-1, TAG_UPDATE, MPI_COMM_WORLD, &requests[request_count++]);
    }

    // Send bottom row (row height-2) to rank+1's top halo (row 0)
    // Receive from rank+1 into bottom halo (row height-1)
    if (rank < size-1) {
        MPI_Isend(b->tiles[b->height-2], b->width, MPI_INT, rank+1, TAG_UPDATE, MPI_COMM_WORLD, &requests[request_count++]);
        MPI_Irecv(b->tiles[b->height-1], b->width, MPI_INT, rank+1, TAG_UPDATE, MPI_COMM_WORLD, &requests[request_count++]);
    }

    MPI_Waitall(request_count, requests, MPI_STATUSES_IGNORE);
}

// Stabilize the local portion of the board with halos, print at each iteration
int stabilizeBoard(struct Board_square *b, int rank, int size) {
    int changed, global_changed;
    int iter = 0;

    do {
        changed = 0;

        // Exchange boundary halos first
        exchangeBoundaries(b, rank, size);

        // Print board after exchange boundaries
        printLocalBoardWithHalos(b, rank, iter);

        // Topple only inner cells (exclude halos)
        for (int i = 1; i < b->height - 1; i++) {
            for (int j = 0; j < b->width; j++) {
                if (async_topple(b, i, j)) {
                    changed = 1;
                }
            }
        }

        // Print board after toppling step
        printLocalBoardWithHalos(b, rank, iter);

        // Check global change
        MPI_Allreduce(&changed, &global_changed, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

        iter++;
    } while (global_changed);

    return 0;
}

// Initialize local portion of the board including halos (halos zero)
void initializeBoardWithHalos(struct Board_square *b, int value) {
    for (int i = 0; i < b->height; i++) {
        for (int j = 0; j < b->width; j++) {
            b->tiles[i][j].value = (i == 0 || i == b->height - 1) ? 0 : value; // halos zero
        }
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct Board_square board;
    int global_width, global_height;

    if (rank == 0) {
        printf("Enter board width: ");
        fflush(stdout);
        scanf("%d", &global_width);
        printf("Enter board height: ");
        fflush(stdout);
        scanf("%d", &global_height);
    }

    MPI_Bcast(&global_width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&global_height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Add 2 rows for halos (top and bottom)
    int local_height = global_height / size;
    int remainder = global_height % size;
    if (rank < remainder) local_height++;
    local_height += 2; // add 2 halo rows

    board.width = global_width;
    board.height = local_height;
    board.threshold = 4;

    // Allocate local grid with halos
    board.tiles = malloc(sizeof(struct Tile_square *) * local_height);
    for (int i = 0; i < local_height; i++) {
        board.tiles[i] = malloc(sizeof(struct Tile_square) * global_width);
    }

    // Initialize board (halos zero, inner rows with 4)
    initializeBoardWithHalos(&board, 4);

    if (rank == 0) printf("--- Initial Board with halos ---\n");
    printLocalBoardWithHalos(&board, rank, 0);

    clock_t start = clock();
    stabilizeBoard(&board, rank, size);
    clock_t end = clock();

    if (rank == 0) {
        double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Time taken to stabilize board: %.10f seconds\n", time_taken);
    }

    if (rank == 0) printf("--- Stabilized Board with halos ---\n");
    printLocalBoardWithHalos(&board, rank, -1);

    writeBoardToFile(&board, "board.txt", rank, size);
    if (rank == 0) printf("Board written to board.txt\n");

    for (int i = 0; i < local_height; i++) {
        free(board.tiles[i]);
    }
    free(board.tiles);

    MPI_Finalize();
    return 0;
}
