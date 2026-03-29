#ifndef MAP_H
#define MAP_H

#include <math.h>
#include <graphx.h>
#include "globals.h"

#define MAP Tile map[15][20]

typedef struct {
    int type;
    int rotation;
} Tile;

void drawTile(Tile t, int x, int y, int showTestTiles);
void generateMap(MAP, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void loadRNGMap(MAP, int seed, int iterations, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles);
void drawMap(MAP, int showTestTiles);
void mapSprite(gfx_sprite_t sprite, MAP);

#endif // MAP_H