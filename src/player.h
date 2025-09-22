#ifndef PLAYER_H
#define PLAYER_H

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

void drawPlayer(struct Player p);
void getPlayerTilePos(struct Player p, int tilePos[2]);
bool playerTouchingColor(struct Player p, uint8_t color);
void playerMovement(struct Player *p);
void resetPlayer(struct Player *p, int spawnX, int spawnY);
void debugPlayerPosition(struct Player p);

#endif // PLAYER_H