#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define TILE_SIZE 16
#define GRAVITY   -0.8
#define SPEED     0.85
#define FRICTION  0.8
#define JUMP      7.7

struct Player {
    float x;
    float y;
    float dx;
    float dy;
};

void drawPlayer(float x,float y) {
    gfx_SetColor(0xE0); // red
    gfx_FillRectangle((int)x, (int)y, (int)(TILE_SIZE/2)-1, (int)(TILE_SIZE/2)-1);
}

// returns in (x, y) format
void getPlayerTilePos(int playerTilePos[2], struct Player p) {
    playerTilePos[0] = (int)(p.x / 24);
    playerTilePos[1] = (int)(p.y / 18);
}

// the 5 values are above, left, center, right, bottom
void getPlayerCollisions(bool collisions[5], int map[18][24], struct Player p) {
    // get the tile the player is on
    int playerTilePos[2];
    getPlayerTilePos(playerTilePos, p);

    if (playerTilePos[1] > 0) collisions[1] = map[playerTilePos[1]-1][playerTilePos[0]] != 9;     // above
    if (playerTilePos[0] > 0) collisions[0] = map[playerTilePos[1]][playerTilePos[0]-1] != 9;     // left
    collisions[2] = map[playerTilePos[1]][playerTilePos[0]] != 9;                                 // center
    if (playerTilePos[0] < 24) collisions[3] = map[playerTilePos[1]][playerTilePos[0]+1] != 9;    // right
    if (playerTilePos[1] < 18) collisions[4] = map[playerTilePos[1]+1][playerTilePos[0]] != 9;    // bottom
}

bool any(bool array[], int size) {
    bool res = false;
    for (int i = 0; i < size; i++) {
        if ((bool) array[i]) {
            res = true;
            break;
        }
    }

    return res;
}

void drawTile(int x, int y, int type) {
    switch (type) {
        case 0: // full block
            gfx_SetColor(0x00); // black
            gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE);
            break;
        case 1: // stair
        case 2: // slab
        case 3: // slope
        case 4: // spike
        case 5: // quarter block
        case 6: // diagonal slab
        default: // air (id 9)
            break;
    }
}

void generateMap(int map[18][24], int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety) {
    for (int y = 0; y < GFX_LCD_HEIGHT / TILE_SIZE; y++) {
        for (int x = 0; x < GFX_LCD_WIDTH / TILE_SIZE; x++) {
            if ((((int)rand() * ((int)log(y*TILE_SIZE) * 170 + 170 + abs(caveHeight-500))) + 1) > (wsChance * 6) || ((x*TILE_SIZE == spawnX) && (y*TILE_SIZE == spawnY - TILE_SIZE)))
                map[y][x] = 0;
            else map[y][x] = 9;
        }
    }
}

void drawMap(int map[18][24]) {
    for (int y = 0; y < GFX_LCD_HEIGHT / TILE_SIZE; y++) {
        for (int x = 0; x < GFX_LCD_WIDTH / TILE_SIZE; x++) {
            drawTile(x*TILE_SIZE, y*TILE_SIZE, map[y][x]);
        }
    }
}

int main() {
    // Initialize the graphics
    gfx_Begin();
    gfx_SetDrawBuffer();

    // initialize font
    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(0x03); // green

    // initialize random
    srand(rtc_Time());

    bool debug = true;

    // generate level
    int spawnX = TILE_SIZE/2;
    int spawnY = GFX_LCD_HEIGHT/2 + TILE_SIZE/2;
    int caveHeight = 0;
    int wsChance = 60;
    int blockVariety = 75;
    int map[18][24];
    
    generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety);

    // initialize the player
    struct Player p;
    p.x = (float)spawnX;
    p.y = (float)spawnY;

    // Main game loop
    while (1) {
        kb_Scan();  // Scan the keypad

        if (kb_IsDown(kb_KeyClear)) { // Exit if Clear is pressed
            break;
        }

        gfx_FillScreen(0xFF);

        // generate map
        drawMap(map);

        // player control
        bool collisions[5];
        bool colliding;

        // handle y movement
        p.dy -= GRAVITY;
        p.y -= p.dy;
        getPlayerCollisions(collisions, map, p);
        colliding = any(collisions, 5);
        if (colliding) {
            while (any(collisions, 5)) {
                p.y -= -(fabsf(p.dy) / p.dy);
                getPlayerCollisions(collisions, map, p);
            }
            p.dy = JUMP * (float)(kb_IsDown(kb_KeyUp) && -(fabsf(p.dy) / p.dy));
        }

        // handle x movement
        p.dx = FRICTION * (p.dx + SPEED * (float)kb_IsDown(kb_KeyRight) - SPEED * (float)kb_IsDown(kb_KeyLeft));
        p.x += p.dx;
        getPlayerCollisions(collisions, map, p);
        colliding = any(collisions, 5);
        if (colliding) {
            while (any(collisions, 5)) {
                p.x += -(fabsf(p.dx) / p.dx);
                getPlayerCollisions(collisions, map, p);
            }
            if (kb_IsDown(kb_KeyUp)) {
                p.dy = JUMP;
                p.dx = -4.0 * (fabsf(p.dx) / p.dx);
            }
        }

        
        if (kb_IsDown(kb_KeyAlpha)) {
            generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety);

            p.x = spawnX;
            p.y = spawnY;
            p.dx = 0;
            p.dy = 0;

            continue;
        }

        drawPlayer(p.x, p.y);

        if (kb_IsDown(kb_KeyMode)) debug = !debug;

        if (debug) {
            // display coordinates
            gfx_SetTextXY(0, 0);
            gfx_PrintInt(p.x, 0);
            gfx_SetTextXY(gfx_GetTextX()+8, 0);
            gfx_PrintInt(p.y, 0);

            gfx_SetTextXY(0, 8);
            for (int i = 0; i < 5; i++) {
                gfx_PrintInt(collisions[i], 0);
                gfx_SetTextXY(gfx_GetTextX()+8, 0);
            }
        }

        // Swap buffers to show the updated screen
        gfx_SwapDraw();

        // Delay for a short period
        delay(50);
    }

    // Close the graphics and clean up
    gfx_End();
    return 0;
}
