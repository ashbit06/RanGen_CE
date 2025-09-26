#ifndef MAP_H
#define MAP_H

#include <math.h>
#include <graphx.h>
#include "globals.h"

typedef struct {
    int type;
    int rotation;
} Tile;

void drawTile(Tile t, int x, int y, int showTestTiles);
void generateMap(Tile map[15][20], int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void loadRNGMap(Tile map[15][20], int seed, int iterations, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void drawMap(Tile map[15][20], int showTestTiles);
void mapSprite(gfx_sprite_t sprite, Tile map[15][20]);

#endif // MAP_H