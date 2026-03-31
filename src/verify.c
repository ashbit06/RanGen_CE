#include <math.h>
#include "globals.h"
#include "map.h"
#include "queue.h"


/**
 * tiles are stored like this:
 * 
 * 01
 * 32
 */
typedef uint8_t TileMini;

TileMini convertTile(Tile t) {
    TileMini mini;

    switch (t.type) {
        case 1: // full block
            mini = 0b1111;
            break;
        case 2: // stair
            mini = 0b0111;
            break;
        case 3: // slab
            mini = 0b0011;
            break;
        case 4: // slope
            mini = 0b0111;
            break;
        case 5: // spike
            mini = 0b1111;
            break;
        case 6: // quater block
            mini = 0b0100;
            break;
        case 7: // diagonal slab
            mini = 0b1010;
            break;
        case 0: // air
        default:
            mini = 0b0000;
    }

    mini = rotateBits(mini, t.rotation, 4);
    return mini;
}

void convertTiles(TileMini extMap[MAP_HEIGHT][MAP_WIDTH], Tile oldMap[MAP_HEIGHT][MAP_WIDTH]) {
    for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
        for (uint8_t x = 0; x < MAP_WIDTH; x++) {
            extMap[y][x] = convertTile(oldMap[y][x]);
        }
    }
}

void convertMiniTileMap(uint8_t graph[MAP_HEIGHT*2][MAP_WIDTH*2], TileMini extMap[MAP_HEIGHT][MAP_WIDTH]) {
    for (uint8_t y = 0; y < MAP_HEIGHT; y++) {
        for (uint8_t x = 0; x < MAP_WIDTH; x++) {
            graph[y*2][x]     = (extMap[y][x] >> 3) & 0b1;
            graph[y*2][x+1]   = (extMap[y][x] >> 2) & 0b1;
            graph[y*2+1][x+1] = (extMap[y][x] >> 1) & 0b1;
            graph[y*2+1][x]   =  extMap[y][x]       & 0b1;
        }
    }
}

bool verifyMap(Tile map[MAP_HEIGHT][MAP_WIDTH], TilePos spawnTile) {
    TileMini extMap[MAP_HEIGHT][MAP_WIDTH];
    uint8_t graph[MAP_HEIGHT*2][MAP_WIDTH*2];
    uint8_t visited[MAP_HEIGHT*2][MAP_WIDTH*2] = {0};
    uint8_t dx[] = {0, 0, 1, -1};
    uint8_t dy[] = {1, -1, 0, 0};
    TilePos pos;
    uint8_t x, nx, y, ny;

    convertTiles(extMap, map);
    convertMiniTileMap(graph, extMap);

    Queue* path = queue_create();
    queue_enqueue(path, spawnTile);
    visited[UNPACK_Y(spawnTile)][UNPACK_X(spawnTile)] = 1;

    while (!queue_is_empty(path)) {
        pos = queue_dequeue(path);
        x = UNPACK_X(pos);
        y = UNPACK_Y(pos);

        if (x == MAP_WIDTH*2 - 1) {
            queue_free(path);
            return true;
        }

        for (uint8_t i = 0; i < 4; i++) {
            nx = x + dx[i];
            ny = y + dy[i];

            if (nx < MAP_WIDTH*2 && ny < MAP_HEIGHT*2 &&
                    !visited[ny][nx] && graph[ny][nx] == 0) {
                visited[ny][nx] = 1;
                queue_enqueue(path, PACK_POS(nx, ny));
            }
        }
    }

    queue_free(path);
    return false;
}
