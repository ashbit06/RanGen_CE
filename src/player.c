#include <stdbool.h>
#include <math.h>
#include <graphx.h>
#include <keypadc.h>
#include "globals.h"

struct Player {
    float x;
    float y;
    float dx;
    float dy;
    bool canMove;
};

void drawPlayer(struct Player p) {
    gfx_SetColor(0xA0);
    gfx_FillRectangle((int)(p.x - PLAYER_SIZE/2), (int)(p.y - PLAYER_SIZE/2), PLAYER_SIZE, PLAYER_SIZE);
}

void getPlayerTilePos(struct Player p, int tilePos[2]) {
    tilePos[0] = (int)(p.x / TILE_SIZE); // x
    tilePos[1] = (int)(p.y / TILE_SIZE); // y
}

bool playerTouchingColor(struct Player p, uint8_t color) {
    uint8_t tl = gfx_GetPixel((int)(p.x - PLAYER_SIZE/2), (int)(p.y - PLAYER_SIZE/2)); // top left
    uint8_t tr = gfx_GetPixel((int)(p.x + PLAYER_SIZE/2), (int)(p.y - PLAYER_SIZE/2)); // top right
    uint8_t bl = gfx_GetPixel((int)(p.x - PLAYER_SIZE/2), (int)(p.y + PLAYER_SIZE/2)); // bottom left
    uint8_t br = gfx_GetPixel((int)(p.x + PLAYER_SIZE/2), (int)(p.y + PLAYER_SIZE/2)); // bottom right

    if (p.y < 0) { tl = 0xFF; tr = 0xFF; }
    if (p.y > GFX_LCD_HEIGHT) { bl = 0xFF; br = 0xFF; }
    if (p.x < 0) { tl = 0xFF; bl = 0xFF; }
    if (p.x > GFX_LCD_WIDTH) { tr = 0xFF; br = 0xFF; }

    return (tl == color || tr == color || bl == color || br == color);
}

void playerMovement(struct Player *p) {
    // handle vertical movement
    p->dy += GRAVITY;
    p->y -= p->dy;
    if (playerTouchingColor(*p, 0x00)) {
        // floor collision
        int safety = 0;
        while (playerTouchingColor(*p, 0x00) && safety < 50) {
            p->y += fabsf(p->dy) / p->dy;
            safety++;
        }

        // jump
        p->dy = -JUMP * (kb_IsDown(kb_KeyUp) && fabsf(p->dy)/p->dy == -1);
    }

    // handle horizontal movement
    p->dx = FRICTION * (p->dx + SPEED*kb_IsDown(kb_KeyRight) - SPEED*kb_IsDown(kb_KeyLeft)*2);
    p->x += p->dx;
    if (playerTouchingColor(*p, 0x00)) {
        // wall collision
        int safety = 0;
        while (playerTouchingColor(*p, 0x00) && safety < 20) {
            p->x -= fabsf(p->dx) / p->dx;
            safety++;
        }

        // wall jump
        if (kb_IsDown(kb_KeyUp)) {
            p->dy = -JUMP;
            p->dx = fabsf(p->dx) / p->dx * -6;
        }
    }
}

void resetPlayer(struct Player *p, int spawnX, int spawnY) {
    p->x = (float)spawnX - TILE_SIZE/4;
    p->y = (float)spawnY - TILE_SIZE/2 - 1;
    p->dy = 0;
    p->dy = 0;
    p->canMove = true;
}

void debugPlayerPosition(struct Player p) {
    gfx_SetTextXY(0, 0);
    gfx_PrintString("Player Pos: ");
    gfx_PrintInt((int)p.x, 0);
    gfx_PrintString(", ");
    gfx_PrintInt((int)p.y, 0);
}
