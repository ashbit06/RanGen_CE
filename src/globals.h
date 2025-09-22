#ifndef DEFINES_H
#define DEFINES_H

#include <stdbool.h>
#include <string.h>

#define TILE_SIZE  16
#define BG_COLOR    1
#define PLAYER_SIZE 5
#define GRAVITY    -0.64
#define FRICTION    0.64
#define JUMP       -6.16
#define SPEED       0.68

#define DEFAULT_SPAWNX (int)(TILE_SIZE/2 + PLAYER_SIZE/2)
#define DEFAULT_SPAWNY (int)(GFX_LCD_HEIGHT/2 - PLAYER_SIZE/2) - 1
#define DEFAULT_SPAWN_BLOCK     1
#define DEFAULT_CAVE_HEIGHT     0
#define DEFAULT_WS_CHANCE      65
#define DEFAULT_BLOCK_VARIETY  75
#define DEFAULT_SHOW_TEST_TILES 0

struct settings {
    char spawnX[4];
    char spawnY[4];
    char spawnBlock[4];
    char caveHeight[4];
    char wsChance[4];
    char blockVariety[4];
    char showTestTiles[2];
    char allTimeCompleted[16];
};

extern struct settings storedSettings;

bool any(bool array[], int size);
bool all(bool array[], int size);
bool startsWith(const char *str, const char *prefix);

#endif // DEFINES_H