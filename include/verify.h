#ifndef VERIFY_H
#define VERIFY_H

#include <math.h>
#include "globals.h"
#include "map.h"


/**
 * tiles are stored like this:
 * 
 * 01
 * 32
 */
typedef uint8_t TileMini;

TileMini convertTile(Tile t);
void convertTiles(TileMini extMap[MAP_HEIGHT][MAP_WIDTH], Tile oldMap[MAP_HEIGHT][MAP_WIDTH]);
void convertMiniTileMap(uint8_t graph[MAP_HEIGHT*2][MAP_WIDTH*2], TileMini extMap[MAP_HEIGHT][MAP_WIDTH]);
bool verifyMap(Tile map[MAP_HEIGHT][MAP_WIDTH]);


#endif // VERIFY_H