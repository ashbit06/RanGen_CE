#include <math.h>
#include <graphx.h>
#include "globals.h"

typedef struct {
    int type;
    int rotation;
} Tile;

void drawTile(Tile t, int x, int y, int showTestTiles) {
    t.rotation %= 4;
    switch (t.type) {
        case 2: // stair
            gfx_SetColor(0x00);
            if (t.rotation == 0) { // 0
                gfx_FillRectangle(x, y + TILE_SIZE/2, TILE_SIZE, TILE_SIZE/2);
                gfx_FillRectangle(x + TILE_SIZE/2, y, TILE_SIZE/2, TILE_SIZE/2);
            } else if (t.rotation == 1) { // 90
                gfx_FillRectangle(x, y, TILE_SIZE/2, TILE_SIZE);
                gfx_FillRectangle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
            } else if (t.rotation == 2) { // 180
                gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE/2);
                gfx_FillRectangle(x, y + TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
            } else if (t.rotation == 3) { // 270
                gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE/2);
                gfx_FillRectangle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
            }
            break;
        case 3: // slab
            gfx_SetColor(0x00);
            if (t.rotation == 0) gfx_FillRectangle(x, y + TILE_SIZE/2, TILE_SIZE, TILE_SIZE/2);
            else if (t.rotation == 1) gfx_FillRectangle(x, y, TILE_SIZE/2, TILE_SIZE);
            else if (t.rotation == 2) gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE/2);
            else if (t.rotation == 3) gfx_FillRectangle(x + TILE_SIZE/2, y, TILE_SIZE/2, TILE_SIZE);
            break;
        case 6: // quarter block
            gfx_SetColor(0x00);
            if (t.rotation == 0) gfx_FillRectangle(x + TILE_SIZE/2, y, TILE_SIZE/2, TILE_SIZE/2);
            else if (t.rotation == 1) gfx_FillRectangle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
            else if (t.rotation == 2) gfx_FillRectangle(x, y + TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
            else if (t.rotation == 3) gfx_FillRectangle(x, y, TILE_SIZE/2, TILE_SIZE/2); 
            break;
        case 7: // diagonal slab
            gfx_SetColor(0x00);
            if (t.rotation % 2 == 0) {
                gfx_FillRectangle(x, y, TILE_SIZE/2, TILE_SIZE/2);
                gfx_FillRectangle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
            } else {
                gfx_FillRectangle(x + TILE_SIZE/2, y, TILE_SIZE/2, TILE_SIZE/2);
                gfx_FillRectangle(x, y + TILE_SIZE/2, TILE_SIZE/2, TILE_SIZE/2);
            }
            break;
        case 4: { // slope
            if (!showTestTiles) break;

            gfx_SetColor(0x00);
            if (t.rotation == 0) gfx_FillTriangle(x+TILE_SIZE, y, x+TILE_SIZE, y+TILE_SIZE, x, y+TILE_SIZE);
            if (t.rotation == 1) gfx_FillTriangle(x, y, x+TILE_SIZE, y+TILE_SIZE, x, y+TILE_SIZE);
            if (t.rotation == 2) gfx_FillTriangle(x, y, x+TILE_SIZE, y, x, y+TILE_SIZE);
            if (t.rotation == 3) gfx_FillTriangle(x, y, x+TILE_SIZE, y, x+TILE_SIZE, y+TILE_SIZE);
            break;
        }
        case 5: // spike
            if (!showTestTiles) break;

            gfx_SetColor(4);
            if (t.rotation == 0) gfx_FillTriangle(x, y+TILE_SIZE, x+TILE_SIZE, y+TILE_SIZE, x+TILE_SIZE/2, y);
            if (t.rotation == 1) gfx_FillTriangle(x+TILE_SIZE, y+TILE_SIZE, x+TILE_SIZE, y, x, y+TILE_SIZE/2);
            if (t.rotation == 2) gfx_FillTriangle(x, y, x, y+TILE_SIZE, x+TILE_SIZE/2, y);
            if (t.rotation == 3) gfx_FillTriangle(x, y, x, y+TILE_SIZE, x+TILE_SIZE, y+TILE_SIZE/2);
            break;
        case 1: // full block
            gfx_SetColor(0x00); // black
            gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE);
            break;
        case 0:
        default:
            break;
    }
}

void generateMap(Tile map[15][20], int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles) {
    for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 20; x++) {
            map[y][x].rotation = 0;
            if (((rand() % ((int)log(y*TILE_SIZE) * 170 + 170 + abs(caveHeight-500))) + 1) > (wsChance * 6) || ((x*TILE_SIZE == spawnX) && (y*TILE_SIZE == spawnY - TILE_SIZE))) {
                if (rand() % 100 + 1 < blockVariety) {
                    map[y][x].type = rand() % 7 + 1;

                    if (!showTestTiles && (map[y][x].type == 5 || map[y][x].type == 4)) {
                        map[y][x].type = 1;
                        continue;
                    }

                    if (map[y][x].type == 2 || map[y][x].type == 3 || map[y][x].type == 4 || map[y][x].type == 6) // stairs, slabs, slopes, quater blocks
                        map[y][x].rotation = rand() % 4;
                    else if (map[y][x].type == 7) // diagonal slabs
                        map[y][x].rotation = rand() % 2;
                    else if (map[y][x].type == 5) { // spikes
                        // find a direction where the spike is touching a block
                        int direction;
                        for (int i = 0; i < 10; i++) {
                            direction = rand() % 4;
                            uint8_t pixel;
                            switch (direction) {
                                case 0: // up
                                    pixel = gfx_GetPixel(x + TILE_SIZE/2, y + TILE_SIZE + 1); // bottom
                                    break;
                                case 1: // right
                                    pixel = gfx_GetPixel(x - 1, y + TILE_SIZE/2); // left side
                                    break;
                                case 2: // down
                                    pixel = gfx_GetPixel(x + TILE_SIZE/2, y - 1); // top
                                    break;
                                case 3: // left
                                    pixel = gfx_GetPixel(x + TILE_SIZE + 1, y + TILE_SIZE/2); // right side
                                    break;
                            }

                            if (pixel == 0x00) break;
                        }
                        map[y][x].rotation = direction;
                    }
                }
            } else {
                map[y][x].type = 0;  // Empty tile
            }

            // dbg_printf("coords: (%d, %d) \ttype: %d \trotation: %d\n", x, y, map[y][x].type, map[y][x].rotation);
        }
    }

    if (spawnBlock) {
        map[6][0].type = 0;
        map[6][1].type = 0;
        map[7][0].type = 1;
    }
}

/*
take the seed and an integer that is the amount of iterations it took for that specific level to generate
*/
void loadRNGMap(Tile map[15][20], int seed, int iterations, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock, int showTestTiles) {
    srand(seed);
    for (int i = 0; i < iterations; i++) {
        rand();
    }

    generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock, showTestTiles);
}

void drawMap(Tile map[15][20], int showTestTiles) {
    for (int y = 0; y < 18; y++) {
        for (int x = 0; x < 24; x++) {
            drawTile(map[y][x], x * TILE_SIZE, y * TILE_SIZE, showTestTiles);
        }
    }
}

void mapSprite(gfx_sprite_t sprite, Tile map[15][20]) {
    // uint8_t width = 16, height = 16;
    // size_t size = sizeof(gfx_sprite_t) + width * height * sizeof(uint8_t);
    // gfx_sprite_t *sprite = malloc(size);

    // sprite->width = width;
    // sprite->height = height;

    // // draw the map onto the sprite data
    
    drawMap(map, 0);
    for (int i = 0; i < sprite.width * sprite.height; i++) {
        if (sprite.data[i] == BG_COLOR) {
            sprite.data[i] = 0xFF; // set background color to transparent
        }
    }

    for (int j = 0; j < sprite.height; j++) {
        for (int i = 0; i < sprite.width; i++) {
            sprite.data[j * sprite.width + i] = gfx_GetPixel(i, j);
        }
    }

}
