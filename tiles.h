#ifndef TILES_H
#define TILES_H

#include <stdlib.h>
#include <stdint.h>

typedef int bool;
#define TRUE 1
#define FALSE 0

struct Tile_square {
  int value;
};

struct Board_square {
  int width;
  int height;
  int threshold;
  struct Tile_square **tiles;
};

#endif
