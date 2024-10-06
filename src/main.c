#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include <debug.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define TILE_SIZE   16
#define PLAYER_SIZE ((TILE_SIZE - 1) / 2)
#define GRAVITY     -0.8
#define FRICTION    0.8
#define JUMP        -7.7
#define SPEED       0.85

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

void drawPlayer(struct Player p) {
    gfx_SetColor(0xE0);
    gfx_FillRectangle((int)(p.x - PLAYER_SIZE/2), (int)(p.y - PLAYER_SIZE/2), PLAYER_SIZE, PLAYER_SIZE);
}

bool playerTouchingColor(struct Player p, uint8_t color) {
    return gfx_GetPixel((int)p.x, (int)(p.y + PLAYER_SIZE/2) + 1) == color;
}

void playerMovement(struct Player* p) {
    // Apply gravity
    p->dy += GRAVITY;
    p->y -= p->dy;  // Move player by vertical velocity

    p->dx = FRICTION * (p->dx + (SPEED*kb_IsDown(kb_KeyRight)) - SPEED*kb_IsDown(kb_KeyLeft));
    p->x += p->dx;

    // Check if the player is touching the ground
    if (playerTouchingColor(*p, 0x00)) {
        dbg_printf("touching black\n");

        int safety = 0;
        // Ensure player is moved up until not intersecting the ground
        while (playerTouchingColor(*p, 0x00) && safety < 50) {
            p->y += fabsf(p->dy) / p->dy;  // Move player upwards
            safety++;
        }

        safety = 0;
        while(playerTouchingColor(*p, 0x00) && safety < 20) {
            p->x -= fabsf(p->dx) / p->dx;
            safety++;
        }

        p->dy = 0;
        
        // Check for jump input
        if (kb_IsDown(kb_KeyUp)) {
            dbg_printf("jumping\n");
            p->dy = -JUMP;  // Apply upward force for jump
            p->dx = -fabsf(p->dx) / p->dx;
        }
    }
}

void resetPlayer(struct Player* p, int spawnX, int spawnY) {
    p->x = (float)spawnX - TILE_SIZE/4;
    p->y = (float)spawnY - TILE_SIZE/2 - 1;
    p->dy = 0;
    p->dy = 0;
}

void debugPlayerPosition(struct Player p) {
    gfx_SetTextXY(0, 0);
    gfx_PrintString("Player Pos: ");
    gfx_PrintInt((int)p.x, 0);
    gfx_PrintString(", ");
    gfx_PrintInt((int)p.y, 0);

    // gfx_SetTextXY(0, 8);
    // gfx_PrintString("Bottom Collide: ");
    // if (playerTouchingColor(p, 0x00)) gfx_PrintString("0");
    // else gfx_PrintString("0");
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

    int spawnX = TILE_SIZE/2;
    int spawnY = GFX_LCD_HEIGHT/2 + TILE_SIZE/2;
    int caveHeight = 0;
    int wsChance = 60;

    int map[18][24];
    int levelsCompleted = 0;
    
    // generate level
    generateMap(map, spawnX, spawnY, caveHeight, wsChance);

    // initialize the player
    struct Player p;
    resetPlayer(&p, spawnX, spawnY);

    while (1) {
        kb_Scan();
        if (kb_IsDown(kb_KeyClear)) {
            break;
        }

        gfx_FillScreen(0xFF);
        drawMap(map);

        playerMovement(&p);

        if (kb_IsDown(kb_KeyAlpha) || p.y > GFX_LCD_HEIGHT) {
            generateMap(map, spawnX, spawnY, caveHeight, wsChance);
            resetPlayer(&p, spawnX, spawnY);
            continue;
        }

        if (p.x > GFX_LCD_WIDTH) {
            generateMap(map, spawnX, spawnY, caveHeight, wsChance);
            resetPlayer(&p, spawnX, spawnY);
            levelsCompleted++;
            continue;            
        }

        drawPlayer(p);

        debugPlayerPosition(p);
        
        gfx_SetTextXY(0, 8);
        gfx_PrintString("Levels Completed: ");
        gfx_PrintInt(levelsCompleted, 1);

        gfx_SwapDraw();
        delay(50);
    }

    gfx_End();
    return 0;
}