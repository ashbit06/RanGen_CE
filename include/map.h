#ifndef MAP_H
#define MAP_H

#include <math.h>
#include <graphx.h>
#include "globals.h"

#define PACK_POS(x, y) ((x) | ((y) << 8))
#define UNPACK_X(p) ((p) & 0xFF)
#define UNPACK_Y(p) (((p) >> 8) & 0xFF)


typedef uint16_t TilePos;

typedef struct {
    uint8_t type;
    uint8_t rotation;
} Tile;

void drawTile(Tile t, int x, int y, int showTestTiles);
void generateMap(Tile map[MAP_HEIGHT][MAP_WIDTH], int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void loadRNGMap(Tile map[MAP_HEIGHT][MAP_WIDTH], int seed, int iterations, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void drawMap(Tile map[MAP_HEIGHT][MAP_WIDTH], int showTestTiles);
void mapSprite(gfx_sprite_t sprite, Tile map[MAP_HEIGHT][MAP_WIDTH]);

#endif // MAP_H