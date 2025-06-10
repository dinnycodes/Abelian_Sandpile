#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "tiles.h"  


// Write final board to file
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

// Asynchronous topple logic (in-place update)
int async_topple(struct Board_square *b, int y, int x) {
    if (b->tiles[y][x].value < 4) return 0;

    int div4 = b->tiles[y][x].value / 4;

    if (x > 0)           b->tiles[y][x - 1].value += div4;
    if (x < b->width-1)  b->tiles[y][x + 1].value += div4;
    if (y > 0)           b->tiles[y - 1][x].value += div4;
    if (y < b->height-1) b->tiles[y + 1][x].value += div4;

    b->tiles[y][x].value %= 4;

    return 1; // Cell changed
}

// Stabilize the board using asynchronous updates
int stabilizeBoard(struct Board_square *b) {
    int changed;
    do {
        changed = 0;
        for (int i = 0; i < b->height; i++) {
            for (int j = 0; j < b->width; j++) {
                if (async_topple(b, i, j)) {
                    changed = 1;
                }
            }
        }
    } while (changed);
    return 0;
}

// Fill the board with initial value
void initializeBoard(struct Board_square *b, int value) {
    for (int i = 0; i < b->height; i++) {
        for (int j = 0; j < b->width; j++) {
            b->tiles[i][j].value = value;
        }
    }
}

int main() {
    struct Board_square board;
    int width, height;

    printf("Enter board width: ");
    scanf("%d", &width);
    printf("Enter board height: ");
    scanf("%d", &height);

    board.width = width;
    board.height = height;
    board.threshold = 4;

    // Allocate 2D grid of tiles
    board.tiles = malloc(sizeof(struct Tile_square *) * height);
    for (int i = 0; i < height; i++) {
        board.tiles[i] = malloc(sizeof(struct Tile_square) * width);
    }

    // Initialize grid with grains
    initializeBoard(&board, 4);

 
    clock_t start = clock();
    stabilizeBoard(&board);
    clock_t end = clock();

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("Board size: %d x %d\n", width, height);
    printf("Time taken to stabilize board: %.10f seconds\n", time_taken);

    writeBoardToFile(&board, "board.txt");
    writeBoardToFile(&board, "serial.out");
    printf("Board written to board.txt\n");

    // Clean up memory
    for (int i = 0; i < height; i++) {
        free(board.tiles[i]);
    }
    free(board.tiles);

    return 0;
}
