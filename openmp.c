// openmp_very_fast.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include "tiles.h"

#define THRESHOLD 4

void writeBoardToFile(struct Board_square *b, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) { perror("Failed to open output file"); return; }
    for (int i = 0; i < b->height; i++) {
        for (int j = 0; j < b->width; j++) {
            fprintf(fp, "%d ", b->tiles[i][j].value);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

int main() {
    int width, height;
    printf("Enter board width: "); fflush(stdout);
    scanf("%d", &width);
    printf("Enter board height: "); fflush(stdout);
    scanf("%d", &height);

    const int N = width * height;
    int numt = omp_get_max_threads();

    // precompute row‐ranges for each thread
    int *start = malloc(numt * sizeof(int));
    int *count = malloc(numt * sizeof(int));
    int base = height / numt, rem = height % numt, off = 0;
    for (int t = 0; t < numt; t++) {
        count[t] = base + (t < rem);
        start[t] = off;
        off += count[t];
    }

    // two flat grids
    int *grid      = malloc(N * sizeof(int));
    int *grid_next = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) grid[i] = THRESHOLD;

    // per‐thread halo buffers
    int **send_up   = malloc(numt * sizeof(int*));
    int **send_down = malloc(numt * sizeof(int*));
    for (int t = 0; t < numt; t++) {
        send_up[t]   = calloc(width, sizeof(int));
        send_down[t] = calloc(width, sizeof(int));
    }

    double t0 = omp_get_wtime();
    int global_changed;

    #pragma omp parallel shared(grid, grid_next, send_up, send_down, global_changed)
    {
        int tid = omp_get_thread_num();
        int sr  = start[tid];
        int rc  = count[tid];

        while (1) {
            // reset the “did anything change?” flag
            #pragma omp single
            global_changed = 0;

            // zero out the next grid
            #pragma omp for schedule(static)
            for (int i = 0; i < N; i++) grid_next[i] = 0;

            // clear my halo buffers
            for (int j = 0; j < width; j++) {
                send_up[tid][j] = 0;
                send_down[tid][j] = 0;
            }

            int local_changed = 0;

            // process my block of rows [sr .. sr+rc-1]
            for (int i = sr; i < sr + rc; i++) {
                for (int j = 0; j < width; j++) {
                    int idx = i*width + j;
                    int v   = grid[idx];
                    int d   = v / THRESHOLD;
                    int r   = v % THRESHOLD;
                    grid_next[idx] += r;

                    if (d) {
                        local_changed = 1;
                        // left/right always in-bounds of flat grid
                        if (j > 0)         grid_next[idx-1]   += d;
                        if (j < width-1)   grid_next[idx+1]   += d;
                        // up
                        if (i > 0) {
                            if (i == sr) 
                                send_up[tid][j]   += d;
                            else
                                grid_next[idx-width] += d;
                        }
                        // down
                        if (i < height-1) {
                            if (i == sr + rc - 1)
                                send_down[tid][j] += d;
                            else
                                grid_next[idx+width] += d;
                        }
                    }
                }
            }

            // mark if *any* thread toppled something
            if (local_changed) {
                #pragma omp atomic write
                global_changed = 1;
            }
            #pragma omp barrier

            // if nobody toppled, we’re done
            if (!global_changed) break;

            // gather all halo contributions
            #pragma omp single
            {
                for (int t = 0; t < numt; t++) {
                    int s = start[t], c = count[t];
                    // apply “up” from thread t into row (s-1)
                    if (t > 0) {
                        int baseidx = (s-1)*width;
                        for (int j = 0; j < width; j++)
                            grid_next[baseidx+j] += send_up[t][j];
                    }
                    // apply “down” from thread t into row (s+c)
                    if (t < numt-1) {
                        int baseidx = (s+c)*width;
                        for (int j = 0; j < width; j++)
                            grid_next[baseidx+j] += send_down[t][j];
                    }
                }
            }
            #pragma omp barrier

            // swap in the new grid
            #pragma omp single
            {
                int *tmp = grid;
                grid      = grid_next;
                grid_next = tmp;
            }
            #pragma omp barrier
        }
    }

    double t1 = omp_get_wtime();
    printf("Board size: %d x %d\n", width, height);
    printf("Time to stabilize: %.6f seconds\n", t1 - t0);

    // pack into Board_square and write out
    struct Board_square board;
    board.width     = width;
    board.height    = height;
    board.threshold = THRESHOLD;
    board.tiles     = malloc(height*sizeof(*board.tiles));
    for (int i = 0; i < height; i++) {
        board.tiles[i] = malloc(width*sizeof(*board.tiles[i]));
        for (int j = 0; j < width; j++) {
            board.tiles[i][j].value = grid[i*width + j];
        }
    }

    writeBoardToFile(&board, "board.txt");
    writeBoardToFile(&board, "openmp.out");
    printf("Final board written to board.txt and openmp.out\n");

    // cleanup
    for (int i = 0; i < height; i++) free(board.tiles[i]);
    free(board.tiles);
    for (int t = 0; t < numt; t++) {
        free(send_up[t]);
        free(send_down[t]);
    }
    free(send_up);
    free(send_down);
    free(start);
    free(count);
    free(grid);
    free(grid_next);

    return 0;
}
