#ifndef MAP_H
#define MAP_H

#include <math.h>
#include <graphx.h>
#include "globals.h"

struct Tile {
    int type;
    int rotation;
};

void drawTile(struct Tile t, int x, int y, int showTestTiles);
void generateMap(struct Tile map[15][20], int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void loadRNGMap(struct Tile map[15][20], int seed, int iterations, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void drawMap(struct Tile map[15][20], int showTestTiles);
void mapSprite(gfx_sprite_t sprite, struct Tile map[15][20]);

#endif // MAP_H