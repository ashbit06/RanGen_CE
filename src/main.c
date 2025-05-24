#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#include <tice.h>
#include <debug.h>

#include "appvar_utils.h"

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

// initialize map info
int spawnX = DEFAULT_SPAWNX;
int spawnY = DEFAULT_SPAWNY;
int spawnBlock = DEFAULT_SPAWN_BLOCK;
int caveHeight = DEFAULT_CAVE_HEIGHT;
int wsChance = DEFAULT_WS_CHANCE;
int blockVariety = DEFAULT_BLOCK_VARIETY;
int showTestTiles = DEFAULT_SHOW_TEST_TILES;
int allTimeCompleted = 0;

// initialize stored settings
char stored_SpawnX[4];
char stored_SpawnY[4];
char stored_SpawnBlock[4];
char stored_CaveHeight[4];
char stored_WsChance[4];
char stored_BlockVariety[4];
char stored_ShowTestTiles[2];
char stored_AllTimeCompleted[16];

struct Player {
    float x;
    float y;
    float dx;
    float dy;
    bool canMove;
};

struct Tile {
    int type;
    int rotation;
};

struct Menu {
    bool show;
    char* title;

    int infoLen;
    char** infoList;

    int showOpts; // display the first x options, this wont disable any though
    int optLen;
    char** optList;
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

bool all(bool array[], int size) {
    bool res = true;
    for (int i = 0; i < size; i++) {
        if (!array[i]) {
            res = false;
            break;
        }
    }
    return res;
}

bool startsWith(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}


int adjustParam(int param, int min, int max) {
    int adj = (int)kb_IsDown(kb_KeyRight)/4 - (int)kb_IsDown(kb_KeyLeft)/2;
    if (kb_IsDown(kb_KeyLeft) || kb_IsDown(kb_KeyRight)) dbg_printf("adjusting by %d\n", adj);
    int new = param + adj;

    if (new < min || new > max) return param;
    else return new;
}

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


void drawTile(struct Tile t, int x, int y) {
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

void generateMap(struct Tile map[15][20], int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock) {
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
void loadRNGMap(struct Tile map[15][20], int seed, int iterations, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, int spawnBlock) {
    srand(seed);
    for (int i = 0; i < iterations; i++) {
        rand();
    }

    generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);
}

void drawMap(struct Tile map[15][20]) {
    for (int y = 0; y < 18; y++) {
        for (int x = 0; x < 24; x++) {
            drawTile(map[y][x], x * TILE_SIZE, y * TILE_SIZE);
        }
    }
}


const char* handleMenuMode(struct Menu *menu, const char *menuMode, int selected) {
    dbg_printf("user selected a menu option. selected option %d.\n", selected);
    if (!strcmp(menuMode, "START")) {
        switch (selected)
        {
        case 0: // save
            dbg_printf("running option \"save\"\n");
            break;
        case 1: // load
            dbg_printf("running option \"load\"\n");
            break;
        case 2: // map gegneration
            dbg_printf("running option \"map generation\"\n");
            menuMode = "MAP";
            break;
        case 3: // resume
        default:
            dbg_printf("running option \"resume\"\n");
            menu->show = false;
            break;
        }
    } else if (!strcmp(menuMode, "MAP")) {
        switch (selected) {
        case 0: // save options
            dbg_printf("running option \"save options\"\n");

            gfx_SetColor(0xFF); // white
            gfx_Rectangle(60, 60, 60, 60);
            gfx_SetColor(0x00); // black
            gfx_FillRectangle(61, 61, 59, 59);
            gfx_SetTextFGColor(0xFF); // white
            gfx_PrintStringXY(
                "Saving options...", 60, 60
                // (int)(GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Saving options...")/2),
                // GFX_LCD_HEIGHT/2 - 8
            );

            sprintf(stored_SpawnX, "%d", spawnX);
            sprintf(stored_SpawnY, "%d", spawnY);
            sprintf(stored_SpawnBlock, "%d", spawnBlock);
            sprintf(stored_CaveHeight, "%d", caveHeight);
            sprintf(stored_WsChance, "%d", wsChance);
            sprintf(stored_BlockVariety, "%d", blockVariety);
            sprintf(stored_ShowTestTiles, "%d", showTestTiles);
            
            bool passed[7] = {true, true, true, true, true, true, true};
            if (!writeKeyValue("RanGen", "spawnX", stored_SpawnX))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "spawnY", stored_SpawnY))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "spawnBlock", stored_SpawnBlock))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "caveHeight", stored_CaveHeight))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "wsChance", stored_WsChance))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "blockVariety", stored_BlockVariety))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "showTestTiles", stored_ShowTestTiles))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}

            if (!all(passed, 7)) {
                gfx_FillRectangle(61, 61, 59, 59);
                gfx_PrintStringXY(
                    "Failed to save some options.",
                    (int)(GFX_LCD_WIDTH/2 - gfx_GetStringWidth("Failed to save some options.")/2),
                    GFX_LCD_HEIGHT/2 - 8
                );
            }
            
            break;
        case 1: // reset to defaults
            dbg_printf("running option \"reset to defaults\"\n");
            spawnX = DEFAULT_SPAWNX;
            spawnY = DEFAULT_SPAWNY;
            spawnBlock = DEFAULT_SPAWN_BLOCK;
            caveHeight = DEFAULT_CAVE_HEIGHT;
            wsChance = DEFAULT_WS_CHANCE;
            blockVariety = DEFAULT_BLOCK_VARIETY;
            showTestTiles = DEFAULT_SHOW_TEST_TILES;
            break;
        case 2: // back      
        default:
            dbg_printf("running option \"back\"\n");
            menuMode = "START";
            break;
        }
    }

    return menuMode;
}

void drawSlider(int x, int y, int value, int padding, int min, int max, bool showArrows) {
    gfx_SetTextXY(x, y);

    if (value > min && showArrows) gfx_PrintChar('<');
    else if (min == max && showArrows) gfx_PrintChar('[');
    else gfx_PrintChar(' ');

    gfx_PrintInt(value, padding);

    if (value < max && showArrows) gfx_PrintChar('>');
    else if (min == max && showArrows) gfx_PrintChar(']');
    else gfx_PrintChar(' ');
}

int drawMenu(struct Menu *menu, const char *mode, int selected) {
    // handle user input
    if (kb_IsDown(kb_KeyUp)) selected--;
    else if (kb_IsDown(kb_KeyDown)) selected++;
    
    // handle rollover
    if (selected < 0) selected = menu->optLen-1;
    if (selected >= menu->optLen) selected = 0;
    
    // draw menu window
    gfx_SetColor(0x00); // black
    gfx_FillRectangle(20, 20, 280, 200);
    gfx_SetColor(3); // white border
    gfx_Rectangle(21, 21, 278, 198);

    // title, information, options
    if (!strcmp(mode, "START")) {
        menu->title = "RanGen CE";
        menu->showOpts = true;

        menu->infoLen = 5;
        menu->infoList = malloc(menu->infoLen * sizeof(char*));
        menu->infoList[0] = "version: beta 0.4";
        menu->infoList[1] = "author: ashbit06";
        menu->infoList[2] = "";
        menu->infoList[3] = "A remake of a game I made on Scratch.";
        menu->infoList[4] = "scratch.mit.edu/projects/579486353";

        menu->optLen = 4;
        menu->showOpts = menu->optLen;
        menu->optList = malloc(menu->optLen * sizeof(char*));
        menu->optList[0] = "Save Level";
        menu->optList[1] = "Load Level";
        menu->optList[2] = "Map Generation";
        menu->optList[3] = "Resume";
    }
    else if (!strcmp(mode, "MAP")) {
        menu->title = "Map Generation";

        menu->infoLen = 7;
        menu->infoList = malloc(menu->infoLen * sizeof(char*));
        menu->infoList[0] = "Edit the map generation parameters.";
        menu->infoList[1] = "spawn coords:";
        menu->infoList[2] = "spawn block:";
        menu->infoList[3] = "cave height:";
        menu->infoList[4] = "whitespace chance:";
        menu->infoList[5] = "block variety:";
        menu->infoList[6] = "show test tiles:";

        menu->optLen = 10;
        menu->showOpts = 3;
        menu->optList = malloc(menu->optLen * sizeof(char*));
        menu->optList[0] = "Save options";
        menu->optList[1] = "Reset to defaults";
        menu->optList[2] = "Back";
        menu->optList[3] = "spawnX";
        menu->optList[4] = "spawnY";
        menu->optList[5] = "spawnBlock";
        menu->optList[6] = "caveHeight";
        menu->optList[7] = "wsChance";
        menu->optList[8] = "blockVariety";
        menu->optList[9] = "showTestTiles";

        // print values
        gfx_SetMonospaceFont(8);
        drawSlider(160, 78, spawnX, 3, 0, GFX_LCD_WIDTH, (selected == 3));
        gfx_PrintString(", ");
        drawSlider(gfx_GetTextX(), 78, spawnY, 3, 0, GFX_LCD_HEIGHT, (selected == 4));
        drawSlider(160, gfx_GetTextY() + 10, spawnBlock, 1, 0, 1, (selected == 5));
        drawSlider(160, gfx_GetTextY() + 10, caveHeight, 3, -99, 999, (selected == 6));
        drawSlider(160, gfx_GetTextY() + 10, wsChance, 3, 0, 100, (selected == 7));
        drawSlider(160, gfx_GetTextY() + 10, blockVariety, 3, 0, 100, (selected == 8));
        drawSlider(160, gfx_GetTextY() + 10, showTestTiles, 1, 0, 0, (selected == 9));
        gfx_SetMonospaceFont(0);
    }


    // print title with center alignment
    gfx_SetTextScale(2, 2);
    // dbg_printf("title width: %d\n", gfx_GetStringWidth(menu->title));
    gfx_SetTextXY(GFX_LCD_WIDTH/2 - (int)(gfx_GetStringWidth(menu->title)/2), 36);
    gfx_PrintString(menu->title);

    gfx_SetTextScale(1, 1);

    // print info
    for (int i = 0; i < menu->infoLen; i++) {
        gfx_SetTextXY(28, i*10 + 68);
        gfx_PrintString(menu->infoList[i]);
    }

    // print options
    const int y = gfx_GetTextY(); // + 16;
    // dbg_printf("text y: %d", y);
    if (kb_IsDown(kb_KeyUp) || kb_IsDown(kb_KeyDown)) dbg_printf("selected: %d\n", selected);
    for (int i = 0; i < menu->optLen && i < menu->showOpts; i++) {
        if (i == selected) {
            gfx_SetTextFGColor(0x00); // black
            gfx_SetColor(3);
            gfx_FillRectangle(28, i*10 + y+24, gfx_GetStringWidth(menu->optList[i]), 8);
        } else {
            gfx_SetTextFGColor(3); // white
        }

        gfx_SetTextXY(28, i*10 + y+24);
        gfx_PrintString(menu->optList[i]);
    }

    // free ram
    free(menu->infoList);
    free(menu->optList);

    return selected;
}


int main() {
    int extendDelay = 0; // use to extend the frame delay
    gfx_Begin();

    gfx_palette[1] = gfx_RGBTo1555(222, 222, 222); // background
    gfx_palette[2] = gfx_RGBTo1555(255, 223, 255); 
    gfx_palette[3] = gfx_RGBTo1555(255, 255, 255); // white
    gfx_palette[4] = gfx_RGBTo1555(255,   0,   0); // red
    gfx_SetTransparentColor(3);

    gfx_SetDrawBuffer();
    gfx_SetTextFGColor(2); // white
    
    srand(rtc_Time());

    static struct Tile map[15][20];
    int currentLevel = 1;

    // get file somehow
    

    // ti_var_t file = ti_Open("RanGen", "r");
    // if (file) {
    //     ti_Read(&allTimeCompleted, sizeof(allTimeCompleted), 1, file);
    //     ti_Close(file);
    // } else {
    //     file = ti_Open("RanGen", "w");
    //     if (file) {
    //         ti_Write(&allTimeCompleted, sizeof(allTimeCompleted), 1, file);
    //         ti_Close(file);
    //     }
    // }
    // dbg_printf("all-time completed: %d\n", allTimeCompleted);

    // load stored settings
    if (readKeyValue("RanGen", "spawnX", stored_SpawnX, sizeof(stored_SpawnX))) {
        spawnX = atoi(stored_SpawnX);
        dbg_printf("stored_SpawnX: %d\n", spawnX);
    } else {
        sprintf(stored_SpawnX, "%d", DEFAULT_SPAWNX);
        writeKeyValue("RanGen", "spawnX", stored_SpawnX);
        dbg_printf("reset stored_SpawnX to default\n");
    }
    
    if (readKeyValue("RanGen", "spawnY", stored_SpawnY, sizeof(stored_SpawnY))) {
        spawnY = atoi(stored_SpawnY);
        dbg_printf("stored_SpawnY: %d\n", spawnY);
    } else {
        sprintf(stored_SpawnY, "%d", DEFAULT_SPAWNY);
        writeKeyValue("RanGen", "spawnY", stored_SpawnY);
        dbg_printf("reset stored_SpawnY to default\n");
    }
    
    if (readKeyValue("RanGen", "spawnBlock", stored_SpawnBlock, sizeof(stored_SpawnBlock))) {
        spawnBlock = atoi(stored_SpawnBlock);
        dbg_printf("stored_SpawnBlock: %d\n", spawnBlock);
    } else {
        sprintf(stored_SpawnBlock, "%d", DEFAULT_SPAWN_BLOCK);
        writeKeyValue("RanGen", "spawnBlock", stored_SpawnBlock);
        dbg_printf("reset stored_SpawnBlock to default\n");
    }
    
    if (readKeyValue("RanGen", "caveHeight", stored_CaveHeight, sizeof(stored_CaveHeight))) {
        caveHeight = atoi(stored_CaveHeight);
        dbg_printf("stored_CaveHeight: %d\n", caveHeight);
    } else {
        sprintf(stored_CaveHeight, "%d", DEFAULT_CAVE_HEIGHT);
        writeKeyValue("RanGen", "caveHeight", stored_CaveHeight);
        dbg_printf("reset stored_CaveHeight to default\n");
    }
    
    if (readKeyValue("RanGen", "wsChance", stored_WsChance, sizeof(stored_WsChance))) {
        wsChance = atoi(stored_WsChance);
        dbg_printf("stored_WsChance: %d\n", wsChance);
    } else {
        sprintf(stored_WsChance, "%d", DEFAULT_WS_CHANCE);
        writeKeyValue("RanGen", "wsChance", stored_WsChance);
        dbg_printf("reset stored_WsChance to default\n");
    }
    
    if (readKeyValue("RanGen", "blockVariety", stored_BlockVariety, sizeof(stored_BlockVariety))) {
        blockVariety = atoi(stored_BlockVariety);
        dbg_printf("stored_BlockVariety: %d\n", blockVariety);
    } else {
        sprintf(stored_BlockVariety, "%d", DEFAULT_BLOCK_VARIETY);
        writeKeyValue("RanGen", "blockVariety", stored_BlockVariety);
        dbg_printf("reset stored_BlockVariety to default\n");
    }
    
    if (readKeyValue("RanGen", "showTestTiles", stored_ShowTestTiles, sizeof(stored_ShowTestTiles))) {
        showTestTiles = atoi(stored_ShowTestTiles);
        dbg_printf("stored_ShowTestTiles: %d\n", showTestTiles);
    } else {
        sprintf(stored_ShowTestTiles, "%d", DEFAULT_SHOW_TEST_TILES);
        writeKeyValue("RanGen", "showTestTiles", stored_ShowTestTiles);
        dbg_printf("reset stored_ShowTestTiles to default\n");
    }
    
    if (readKeyValue("RanGen", "allTimeCompleted", stored_AllTimeCompleted, sizeof(stored_AllTimeCompleted))) {
        allTimeCompleted = atoi(stored_AllTimeCompleted);
        dbg_printf("stored_AllTimeCompleted: %d\n", allTimeCompleted);
    } else {
        sprintf(stored_AllTimeCompleted, "%d", allTimeCompleted);
        writeKeyValue("RanGen", "allTimeCompleted", stored_AllTimeCompleted);
        dbg_printf("reset stored_AllTimeCompleted to default\n");
    }

    dbg_printf("spawnX: %d\n", spawnX);
    dbg_printf("spawnY: %d\n", spawnY);
    dbg_printf("spawnBlock: %d\n", spawnBlock);
    dbg_printf("caveHeight: %d\n", caveHeight);
    dbg_printf("wsChance: %d\n", wsChance);
    dbg_printf("blockVariety: %d\n", blockVariety);
    dbg_printf("showTestTiles: %d\n", showTestTiles);
    dbg_printf("allTimeCompleted: %d\n", allTimeCompleted);

    // generate level first level
    generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);

    // initialize the player
    struct Player player;
    resetPlayer(&player, spawnX, spawnY);

    struct Menu menu;
    menu.show = false;
    const char *menuMode = "START";
    int selected = 0;

    while (1) {
        kb_Scan();
        if (kb_IsDown(kb_KeyClear)) {
            if (menu.show) {
                menu.show = false;
            } else {
                break;
            }
        }

        gfx_FillScreen(1); // the classic gray background :)

        // display level information behind the level
        gfx_SetTextFGColor(3); // white
        gfx_SetTextXY(32, 120);
        gfx_SetTextScale(2, 2);
        gfx_PrintString("LEVEL");
        const int textX = gfx_GetTextX();
        gfx_SetTextXY(textX, 104);
        gfx_SetTextScale(4, 4);
        gfx_PrintInt(currentLevel, 1);
        gfx_SetTextScale(1, 1);
        gfx_SetTextXY(textX, 136);
        gfx_PrintInt(allTimeCompleted, 1);

        drawMap(map);

        if (player.canMove) {
            selected = 0;
            playerMovement(&player);


            if (kb_IsDown(kb_KeyAlpha)) {
                generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);
                resetPlayer(&player, spawnX, spawnY);
                continue;
            }

            if (player.x > GFX_LCD_WIDTH) {
                generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);
                resetPlayer(&player, spawnX, spawnY);
                currentLevel++;
                allTimeCompleted++;
                continue;            
            }

            if (player.y > GFX_LCD_HEIGHT || playerTouchingColor(player, 4) || kb_IsDown(kb_Key2nd)) {
                resetPlayer(&player, spawnX, spawnY);
            }
        }

        // display menu
        if (kb_IsDown(kb_KeyMode)) {
            // toggle menu and set canMove to the opposite of the menu toggle
            menu.show = !menu.show;
            player.canMove = !menu.show;
            selected = 0;
            dbg_printf("mode key pressed, new menu toggle value is %d\n", menu.show);
        }

        drawPlayer(player);
        debugPlayerPosition(player);

        if (menu.show) {
            // handle key presses
            selected = drawMenu(&menu, menuMode, selected);

            if (!strcmp(menuMode, "MAP") && selected >= 2) {
                switch (selected) {
                case 3:
                    spawnX = adjustParam(spawnX, 0, GFX_LCD_WIDTH);
                    break;
                case 4:
                    spawnY = adjustParam(spawnY, 0, GFX_LCD_HEIGHT);
                    break;
                case 5:
                    spawnBlock = adjustParam(spawnBlock, 0, 1);
                    break;
                case 6:
                    caveHeight = adjustParam(caveHeight, -99, 999);
                    break;
                case 7:
                    wsChance = adjustParam(wsChance, 0, 100);
                    break;
                case 8:
                    blockVariety = adjustParam(blockVariety, 0, 100);
                    break;
                case 9:
                    showTestTiles = adjustParam(showTestTiles, 0, 0);
                    break;
                }
            } else if (kb_IsDown(kb_Key2nd)) {
                menuMode = handleMenuMode(&menu, menuMode, selected);
                if (selected == 0) extendDelay = 500;
                selected = 0;
            }
        } else {
            player.canMove = true; // temp fix
        }

        gfx_SwapDraw();
        delay(50 + extendDelay);
        extendDelay = 0;
    }

    // save all-time completed
    // file = ti_Open("RanGen", "w");
    // if (file) {
    //     ti_Write(&allTimeCompleted, sizeof(allTimeCompleted), 1, file);
    //     ti_Close(file);
    // }
    sprintf(stored_AllTimeCompleted, "%d", allTimeCompleted);
    if (!writeKeyValue("RanGen", "allTimeCompleted", stored_AllTimeCompleted)) {
        dbg_printf("failed to save allTimeCompleted on close\n");
        gfx_FillScreen(0xFF);
        gfx_PrintStringXY("failed to save allTimeCompleted on close\n", 0, 0);
        gfx_SwapDraw();

        while(!os_GetCSC()) delay(50);
    };

    gfx_End();
    return 0;
}