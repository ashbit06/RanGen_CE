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

#include "globals.h"
#include "appvar_utils.h"
#include "menu.h"

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

// initialize map info
int spawnX = DEFAULT_SPAWNX;
int spawnY = DEFAULT_SPAWNY;
int spawnBlock = DEFAULT_SPAWN_BLOCK;
int caveHeight = DEFAULT_CAVE_HEIGHT;
int wsChance = DEFAULT_WS_CHANCE;
int blockVariety = DEFAULT_BLOCK_VARIETY;
int showTestTiles = DEFAULT_SHOW_TEST_TILES;
int allTimeCompleted = 0;

struct MenuData makeMenuData() {
    struct MenuData data;
    data.spawnX = spawnX;
    data.spawnY = spawnY;
    data.spawnBlock = spawnBlock;
    data.caveHeight = caveHeight;
    data.wsChance = wsChance;
    data.blockVariety = blockVariety;
    data.showTestTiles = showTestTiles;
    return data;
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

void mapSprite(gfx_sprite_t sprite, struct Tile map[15][20]) {
    // uint8_t width = 16, height = 16;
    // size_t size = sizeof(gfx_sprite_t) + width * height * sizeof(uint8_t);
    // gfx_sprite_t *sprite = malloc(size);

    // sprite->width = width;
    // sprite->height = height;

    // // draw the map onto the sprite data
    
    drawMap(map);
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
    
    // load stored settings
    if (readKeyValue("RanGen", "spawnX", storedSettings.spawnX, sizeof(storedSettings.spawnX))) {
        spawnX = atoi(storedSettings.spawnX);
        dbg_printf("stored_SpawnX: %d\n", spawnX);
    } else {
        sprintf(storedSettings.spawnX, "%d", DEFAULT_SPAWNX);
        writeKeyValue("RanGen", "spawnX", storedSettings.spawnX);
        dbg_printf("reset stored_SpawnX to default\n");
    }
    
    if (readKeyValue("RanGen", "spawnY", storedSettings.spawnY, sizeof(storedSettings.spawnY))) {
        spawnY = atoi(storedSettings.spawnY);
        dbg_printf("stored_SpawnY: %d\n", spawnY);
    } else {
        sprintf(storedSettings.spawnY, "%d", DEFAULT_SPAWNY);
        writeKeyValue("RanGen", "spawnY", storedSettings.spawnY);
        dbg_printf("reset stored_SpawnY to default\n");
    }
    
    if (readKeyValue("RanGen", "spawnBlock", storedSettings.spawnBlock, sizeof(storedSettings.spawnBlock))) {
        spawnBlock = atoi(storedSettings.spawnBlock);
        dbg_printf("stored_SpawnBlock: %d\n", spawnBlock);
    } else {
        sprintf(storedSettings.spawnBlock, "%d", DEFAULT_SPAWN_BLOCK);
        writeKeyValue("RanGen", "spawnBlock", storedSettings.spawnBlock);
        dbg_printf("reset stored_SpawnBlock to default\n");
    }
    
    if (readKeyValue("RanGen", "caveHeight", storedSettings.caveHeight, sizeof(storedSettings.caveHeight))) {
        caveHeight = atoi(storedSettings.caveHeight);
        dbg_printf("stored_CaveHeight: %d\n", caveHeight);
    } else {
        sprintf(storedSettings.caveHeight, "%d", DEFAULT_CAVE_HEIGHT);
        writeKeyValue("RanGen", "caveHeight", storedSettings.caveHeight);
        dbg_printf("reset stored_CaveHeight to default\n");
    }
    
    if (readKeyValue("RanGen", "wsChance", storedSettings.wsChance, sizeof(storedSettings.wsChance))) {
        wsChance = atoi(storedSettings.wsChance);
        dbg_printf("stored_WsChance: %d\n", wsChance);
    } else {
        sprintf(storedSettings.wsChance, "%d", DEFAULT_WS_CHANCE);
        writeKeyValue("RanGen", "wsChance", storedSettings.wsChance);
        dbg_printf("reset stored_WsChance to default\n");
    }
    
    if (readKeyValue("RanGen", "blockVariety", storedSettings.blockVariety, sizeof(storedSettings.blockVariety))) {
        blockVariety = atoi(storedSettings.blockVariety);
        dbg_printf("stored_BlockVariety: %d\n", blockVariety);
    } else {
        sprintf(storedSettings.blockVariety, "%d", DEFAULT_BLOCK_VARIETY);
        writeKeyValue("RanGen", "blockVariety", storedSettings.blockVariety);
        dbg_printf("reset stored_BlockVariety to default\n");
    }
    
    if (readKeyValue("RanGen", "showTestTiles", storedSettings.showTestTiles, sizeof(storedSettings.showTestTiles))) {
        showTestTiles = atoi(storedSettings.showTestTiles);
        dbg_printf("stored_ShowTestTiles: %d\n", showTestTiles);
    } else {
        sprintf(storedSettings.showTestTiles, "%d", DEFAULT_SHOW_TEST_TILES);
        writeKeyValue("RanGen", "showTestTiles", storedSettings.showTestTiles);
        dbg_printf("reset stored_ShowTestTiles to default\n");
    }
    
    if (readKeyValue("RanGen", "allTimeCompleted", storedSettings.allTimeCompleted, sizeof(storedSettings.allTimeCompleted))) {
        allTimeCompleted = atoi(storedSettings.allTimeCompleted);
        dbg_printf("stored_AllTimeCompleted: %d\n", allTimeCompleted);
    } else {
        sprintf(storedSettings.allTimeCompleted, "%d", allTimeCompleted);
        writeKeyValue("RanGen", "allTimeCompleted", storedSettings.allTimeCompleted);
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
    // gfx_sprite_t *sprite = malloc(sizeof(gfx_sprite_t) + pow(TILE_SIZE,2) * sizeof(uint8_t));
    generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);
    // mapSprite(*sprite, map);

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

        // gfx_TransparentSprite(sprite, 0, 0);
        drawMap(map);

        if (player.canMove) {
            selected = 0;
            playerMovement(&player);


            if (kb_IsDown(kb_KeyAlpha)) {
                generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);
                // mapSprite(*sprite, map);
                resetPlayer(&player, spawnX, spawnY);
                continue;
            }

            if (player.x > GFX_LCD_WIDTH) {
                generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);
                // mapSprite(*sprite, map);
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
            struct MenuData data = makeMenuData();
            selected = drawMenu(&menu, &data, menuMode, selected);

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
                menuMode = handleMenuMode(&menu, &data, menuMode, selected);
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
    sprintf(storedSettings.allTimeCompleted, "%d", allTimeCompleted);
    if (!writeKeyValue("RanGen", "allTimeCompleted", storedSettings.allTimeCompleted)) {
        dbg_printf("failed to save allTimeCompleted on close\n");
        gfx_FillScreen(0xFF);
        gfx_PrintStringXY("failed to save allTimeCompleted on close\n", 0, 0);
        gfx_SwapDraw();

        while(!os_GetCSC()) delay(50);
    } else dbg_printf("successfully saved allTimeCompleted\n");

    // free(sprite);
    gfx_End();
    return 0;
}