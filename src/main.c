#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define TILE_SIZE   16
#define PLAYER_SIZE ((int)(TILE_SIZE/2)-1)
#define GRAVITY     0.8
#define SPEED       0.85
#define FRICTION    0.8
#define JUMP        7.7

struct Player {
    float x;
    float y;
    float dx;
    float dy;
};

bool any(bool array[], int size) {
    bool res = false;
    for (int i = 0; i < size; i++) {
        if (array[i]) {
            res = true;
            break;
        }
    }
    return res;
}

void drawPlayer(float x, float y) {
    gfx_SetColor(0xE0); // red
    gfx_FillRectangle((int)x, (int)y, PLAYER_SIZE, PLAYER_SIZE);
}

void getPlayerTilePos(int playerTilePos[2], struct Player p) {
    playerTilePos[0] = (int)(p.x / TILE_SIZE);
    playerTilePos[1] = (int)(p.y / TILE_SIZE);
}

bool isPlayerColliding(int map[18][24], struct Player p) {
    // Get the player's tile positions based on its bounding box
    int leftTile = (int)(p.x / TILE_SIZE);
    int rightTile = (int)((p.x + PLAYER_SIZE) / TILE_SIZE);
    int topTile = (int)(p.y / TILE_SIZE);
    int bottomTile = (int)((p.y + PLAYER_SIZE) / TILE_SIZE);

    // Check the tiles that the player's bounding box overlaps
    // We are considering the four corners of the player's bounding box
    return (map[topTile][leftTile] != 9 ||  // Top-left corner
            map[topTile][rightTile] != 9 ||  // Top-right corner
            map[bottomTile][leftTile] != 9 ||  // Bottom-left corner
            map[bottomTile][rightTile] != 9);  // Bottom-right corner
}

void handlePlayerMovement(struct Player* p, int map[18][24]) {
    // Apply gravity
    p->dy += GRAVITY; // Add gravity effect to vertical speed
    p->y += p->dy; // Update vertical position

    // Check if the player is touching black
    if (isPlayerColliding(map, *p)) {
        // Collision detected, reset vertical position and speed
        p->y -= p->dy; // Move the player up to the collision point
        p->dy = 0; // Reset vertical speed
    }

    // Horizontal movement
    p->dx = FRICTION * (p->dx + SPEED * (float)kb_IsDown(kb_KeyRight) - SPEED * (float)kb_IsDown(kb_KeyLeft));
    p->x += p->dx; // Update horizontal position

    // Check if the player is touching black after horizontal movement
    if (isPlayerColliding(map, *p)) {
        // Collision detected, reset horizontal position and speed
        p->x -= p->dx; // Move the player back to the collision point
        p->dx = 0; // Reset horizontal speed
    }

    // Handle jumping
    if (kb_IsDown(kb_KeyUp) && isPlayerColliding(map, *p)) { // If on ground (touching black)
        p->dy = -JUMP; // Jump by applying an upward velocity
    }
}

void debugPlayerPosition(struct Player p) {
    gfx_SetTextXY(0, 0);
    gfx_PrintString("Player Pos: ");
    gfx_PrintInt((int)p.x, 0);
    gfx_PrintString(", ");
    gfx_PrintInt((int)p.y, 0);
}

void drawTile(int x, int y, int type) {
    if (type == 0) {
        gfx_SetColor(0x00); // black
        gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE);
    }
}

void generateMap(int map[18][24], int spawnX, int spawnY, int caveHeight, int wsChance) {
    for (int y = 0; y < GFX_LCD_HEIGHT / TILE_SIZE; y++) {
        for (int x = 0; x < GFX_LCD_WIDTH / TILE_SIZE; x++) {
            if ((((int)rand() * ((int)log(y*TILE_SIZE) * 170 + 170 + abs(caveHeight-500))) + 1) > (wsChance * 6) || ((x*TILE_SIZE == spawnX) && (y*TILE_SIZE == spawnY - TILE_SIZE)))
                map[y][x] = 0;
            else map[y][x] = 9;
        }
    }
}

void drawMap(int map[18][24]) {
    for (int y = 0; y < 18; y++) {
        for (int x = 0; x < 24; x++) {
            drawTile(x * TILE_SIZE, y * TILE_SIZE, map[y][x]);
        }
    }
}

int main() {
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_SetTextFGColor(0x03);
    
    srand(rtc_Time());

    // generate level
    int spawnX = TILE_SIZE/2;
    int spawnY = GFX_LCD_HEIGHT/2 + TILE_SIZE/2;
    int caveHeight = 0;
    int wsChance = 60;
    int map[18][24];
    
    generateMap(map, spawnX, spawnY, caveHeight, wsChance);

    // initialize the player
    struct Player p;
    p.x = (float)spawnX - TILE_SIZE/4;
    p.y = (float)spawnY - TILE_SIZE/2 - 1;

    while (1) {
        kb_Scan();
        if (kb_IsDown(kb_KeyClear)) {
            break;
        }

        if (kb_IsDown(kb_KeyAlpha)) {
            generateMap(map, spawnX, spawnY, caveHeight, wsChance);

            p.x = (float)spawnX - TILE_SIZE/4;
            p.y = (float)spawnY - TILE_SIZE/2 - 1;
            p.dy = 0;
            p.dy = 0;

            continue;
        }

        gfx_FillScreen(0xFF);

        drawMap(map);
        drawPlayer(p.x, p.y);
        debugPlayerPosition(p);

        handlePlayerMovement(&p, map);

        gfx_SwapDraw();
        delay(50);
    }

    gfx_End();
    return 0;
}