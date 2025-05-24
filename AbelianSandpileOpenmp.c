#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tiles.h"
#include <time.h>
#include <omp.h>

void printBoard(struct Board_square *b){
    for (int i = 0; i < b->height; i++){
        printf("[");
        for (int j = 0; j < b->width; j++){
            printf(" %d ", b->tiles[i][j].value);
            if (j < b->width - 1) printf(",");
        }
        printf("]\n");
    }
    printf("\n");
}

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

// Keep the original overflow function - no changes needed
void overflowSq(struct Board_square *b, int x, int y, struct Tile_square **buffer){
    int grains_to_give = b->tiles[x][y].value / 4;
    int remainder = b->tiles[x][y].value % 4;

    if (x > 0)           buffer[x - 1][y].value += grains_to_give;
    if (x < b->height-1) buffer[x + 1][y].value += grains_to_give;
    if (y > 0)           buffer[x][y - 1].value += grains_to_give;
    if (y < b->width-1)  buffer[x][y + 1].value += grains_to_give;

    buffer[x][y].value += remainder;
}

int stabilizeBoard(struct Board_square *b){
    int changed;
    do {
        changed = 0;

        // Create a new buffer board with all values 0 (parallelize allocation)
        struct Tile_square **buffer = malloc(sizeof(struct Tile_square*) * b->height);
        #pragma omp parallel for
        for (int i = 0; i < b->height; i++) {
            buffer[i] = calloc(b->width, sizeof(struct Tile_square));
        }

        // SERIAL processing of overflow logic to maintain correctness
        for (int i = 0; i < b->height; i++) {
            for (int j = 0; j < b->width; j++) {
                if (b->tiles[i][j].value >= 4) {
                    overflowSq(b, i, j, buffer);
                    changed = 1;
                } else {
                    buffer[i][j].value += b->tiles[i][j].value;
                }
            }
        }

        // Copy buffer back into board (parallelize this part)
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < b->height; i++) {
            for (int j = 0; j < b->width; j++) {
                b->tiles[i][j].value = buffer[i][j].value;
            }
        }
        
        // Free buffer (parallelize cleanup)
        #pragma omp parallel for
        for (int i = 0; i < b->height; i++) {
            free(buffer[i]);
        }
        free(buffer);

        //printBoard(b); // Optional: can remove if too verbose
    } while (changed);

    return 0;
}

void initializeBoard(struct Board_square *b, int value){
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < b->height; i++)
        for (int j = 0; j < b->width; j++)
            b->tiles[i][j].value = value;
}

int main(){
    struct Board_square board;
    int width, height;

    printf("Enter board width: ");
    scanf("%d", &width);
    printf("Enter board height: ");
    scanf("%d", &height);

    board.width = width;
    board.height = height;
    board.threshold = 4;

    // Allocate tiles
    board.tiles = malloc(sizeof(struct Tile_square *) * height);
    for (int i = 0; i < height; i++)
        board.tiles[i] = malloc(sizeof(struct Tile_square) * width);

    // Initialize board with value 4
    initializeBoard(&board, 4);

    printf("--- Initial Board ---\n");
    printBoard(&board);

    // Measure time before stabilization using OpenMP timer
    double start = omp_get_wtime();
    
    // Evolve until stable
    stabilizeBoard(&board);

    // Measure time after stabilization
    double end = omp_get_wtime();
    double time_taken = end - start;
    printf("Time taken to stabilize board: %.10f seconds\n", time_taken);

    printf("--- Stabilized Board ---\n");
    printBoard(&board);

    //Print final state to file
    writeBoardToFile(&board, "board.txt");
    printf("Board written to board.txt\n");

    // Free memory
    for (int i = 0; i < height; i++)
        free(board.tiles[i]);
    free(board.tiles);

    return 0;
}