#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include "tiles.h"

// Print the sandpile board
void printBoard(struct Board_square *b) {
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

// Write board to file
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

// Parallel safe toppling
int async_topple(struct Board_square *b, int y, int x) {
    if (b->tiles[y][x].value < 4) return 0;

    int div4 = b->tiles[y][x].value / 4;
    b->tiles[y][x].value %= 4;

    if (x > 0) {
        #pragma omp atomic
        b->tiles[y][x - 1].value += div4;
    }

    if (x < b->width - 1) {
        #pragma omp atomic
        b->tiles[y][x + 1].value += div4;
    }

    if (y > 0) {
        #pragma omp atomic
        b->tiles[y - 1][x].value += div4;
    }

    if (y < b->height - 1) {
        #pragma omp atomic
        b->tiles[y + 1][x].value += div4;
    }

    return 1;
}


// Stabilize board using OpenMP with checkerboard phase-based updates
int stabilizeBoard(struct Board_square *b) {
    int changed;

    do {
        changed = 0;
        for (int phase = 0; phase < 2; phase++) {
            int local_changed = 0;

            #pragma omp parallel for collapse(2) reduction(|:local_changed)
            for (int i = 0; i < b->height; i++) {
                for (int j = 0; j < b->width; j++) {
                    if ((i + j) % 2 == phase) {
                        if (b->tiles[i][j].value >= 4) {
                            if (async_topple(b, i, j)) {
                                local_changed = 1;
                            }
                        }
                    }
                }
            }

            changed |= local_changed;
        }
    } while (changed);

    return 0;
}

// Initialize board with constant value
void initializeBoard(struct Board_square *b, int value) {
    for (int i = 0; i < b->height; i++) {
        for (int j = 0; j < b->width; j++) {
            b->tiles[i][j].value = value;
        }
    }
}

int main() {
    omp_set_num_threads(7);  // Choose a reasonable number based on your CPU

    struct Board_square board;
    int width, height;

    printf("Enter board width: ");
    scanf("%d", &width);
    printf("Enter board height: ");
    scanf("%d", &height);

    board.width = width;
    board.height = height;
    board.threshold = 4;

    board.tiles = malloc(sizeof(struct Tile_square *) * height);
    for (int i = 0; i < height; i++) {
        board.tiles[i] = malloc(sizeof(struct Tile_square) * width);
    }

    initializeBoard(&board, 4);

  //  printf("--- Initial Board ---\n");
  //  printBoard(&board);

    double start = omp_get_wtime();
    stabilizeBoard(&board);
    double end = omp_get_wtime();
    printf("Time taken to stabilize board: %.10f seconds\n", end - start);

  //  printf("--- Stabilized Board ---\n");
  //  printBoard(&board);

    writeBoardToFile(&board, "board.txt");
    printf("Board written to board.txt\n");

    for (int i = 0; i < height; i++) {
        free(board.tiles[i]);
    }
    free(board.tiles);

    return 0;
}