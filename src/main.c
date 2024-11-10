#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#include <tice.h>
#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define TILE_SIZE   16
#define BG_COLOR    1
#define PLAYER_SIZE 5
#define GRAVITY     -0.64
#define FRICTION    0.64
#define JUMP        -6.16
#define SPEED       0.68

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
    p->dx = FRICTION * (p->dx + SPEED*kb_IsDown(kb_KeyRight) - SPEED*kb_IsDown(kb_KeyLeft));
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
        case 4: // slope
            // break;
        case 5: // spike
            // gfx_SetColor(0xE0);
            // break;
        case 1: // full block
            gfx_SetColor(0x00); // black
            gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE);
            break;
        case 0:
        default:
            break;
    }
}

void generateMap(struct Tile map[15][20], int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, bool spawnBlock) {
    for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 20; x++) {
            map[y][x].rotation = 0;
            if (((rand() % ((int)log(y*TILE_SIZE) * 170 + 170 + abs(caveHeight-500))) + 1) > (wsChance * 6) || ((x*TILE_SIZE == spawnX) && (y*TILE_SIZE == spawnY - TILE_SIZE))) {
                if (rand() % 100 + 1 < blockVariety ) {
                    map[y][x].type = rand() % 7 + 1;
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
void loadRNGMap(struct Tile map[15][20], int seed, int iterations, int spawnX, int spawnY, int caveHeight, int wsChance, int blockVariety, bool spawnBlock) {
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
    if (!strcmp(menuMode, "START")) {
        if (selected == 0) {

        } else if (selected == 1) {

        } else if (selected == 2) {

        } else if (selected == 3) {
            menu->show = false;
        }
    }

    return menuMode;
}

void drawMenu(struct Menu *menu, const char *mode, int selected) {
    // handle user input
    if (kb_IsDown(kb_KeyEnter)) {
        selected = 0;
    }
    if (kb_IsDown(kb_KeyUp)) selected--;
    if (kb_IsDown(kb_KeyDown)) selected++;
    
    // handle rollover
    if (selected < 0) selected = 0;
    if (selected >= menu->optLen) selected = menu->optLen - 1;
    
    // draw menu window
    gfx_SetColor(0x00); // black
    gfx_FillRectangle(20, 20, 280, 200);
    gfx_SetColor(2); // white border
    gfx_Rectangle(21, 21, 278, 198);

    // title, information, options
    if (!strcmp(mode, "START")) {
        menu->title = "RanGen CE";

        menu->infoLen = 2;
        menu->infoList = malloc(menu->infoLen * sizeof(char*));
        menu->infoList[0] = "version: beta 0.4";
        menu->infoList[1] = "author: ashbit06";
        menu->infoList[2] = "";
        menu->infoList[3] = "A remake of a game I made on Scratch.";
        menu->infoList[4] = "https://scratch.mit.edu/projects/579486353/";

        menu->optLen = 4;
        menu->optList = malloc(menu->optLen * sizeof(char*));
        menu->optList[0] = "Save Level";
        menu->optList[1] = "Load Level";
        menu->optList[2] = "About";
        menu->optList[3] = "Resume";
    } else if (!strcmp(mode, ""))

    

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
    const int y = gfx_GetTextX();
    dbg_printf("selected: %d\n", selected);
    for (int i = 0; i < menu->optLen; i++) {
        if (i == selected) {
            gfx_SetTextFGColor(0x00); // black
            gfx_SetColor(2);
            gfx_FillRectangle(28, i*10 + y+24, gfx_GetStringWidth(menu->optList[i]), 8);
        } else {
            gfx_SetTextFGColor(2); //white
        }

        gfx_SetTextXY(28, i*10 + y+24);
        gfx_PrintString(menu->optList[i]);
    }

    // free ram
    free(menu->infoList);
    free(menu->optList);
}


int main() {
    gfx_Begin();
    gfx_palette[1] = gfx_RGBTo1555(0xDE, 0xDE, 0xDE); // light gray
    gfx_palette[2] = 0xffdf; // almost white

    gfx_SetDrawBuffer();
    gfx_SetTextFGColor(2); // white
    
    srand(rtc_Time());

    // initialize map info
    int spawnX = TILE_SIZE/2 + PLAYER_SIZE/2;
    int spawnY = GFX_LCD_HEIGHT/2 - PLAYER_SIZE/2;
    int caveHeight = 0;
    int wsChance = 65;
    int blockVariety = 75;
    bool spawnBlock = true;

    struct Tile map[15][20];
    int currentLevel = 1;

    // get file somehow
    int allTimeCompleted = 0;

    ti_var_t file = ti_Open("RanGen", "r");
    if (file) {
        ti_Read(&allTimeCompleted, sizeof(allTimeCompleted), 1, file);
        ti_Close(file);
    } else {
        file = ti_Open("RanGen", "w");
        if (file) {
            ti_Write(&allTimeCompleted, sizeof(allTimeCompleted), 1, file);
            ti_Close(file);
        }
    }
    dbg_printf("all-time completed: %d\n", allTimeCompleted);

    // generate level first level
    generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock);

    // initialize the player
    struct Player player;
    resetPlayer(&player, spawnX, spawnY);

    gfx_sprite_t *behind_sprite = gfx_MallocSprite(GFX_LCD_WIDTH, GFX_LCD_HEIGHT);
    gfx_GetSprite(behind_sprite, 0, 0);

    struct Menu menu;
    menu.show = false;
    const char *menuMode = "START";
    int selected = 0;

    while (1) {
        kb_Scan();
        if (kb_IsDown(kb_KeyClear)) {
            break;
        }

        gfx_FillScreen(1); // the classic gray background :)

        // display level information behind the level
        gfx_SetTextFGColor(2); // white
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

            if (player.y > GFX_LCD_HEIGHT || playerTouchingColor(player, 0xE0) || kb_IsDown(kb_Key2nd)) {
                resetPlayer(&player, spawnX, spawnY);
            }
        }

        // display menu
        if (kb_IsDown(kb_KeyMode)) {
            // toggle menu and set canMove to the opposite of the menu toggle
            menu.show = !menu.show;
            player.canMove = !menu.show;
            dbg_printf("mode key pressed, new menu toggle value is %d\n", menu.show);
        }

        drawPlayer(player);
        debugPlayerPosition(player);

        if (menu.show) {
            // handle key presses
            drawMenu(&menu, menuMode, selected);
            selected += 1;

            if (kb_IsDown(kb_KeyEnter))
                menuMode = handleMenuMode(&menu, menuMode, selected);
        }

        gfx_SwapDraw();
        delay(50);
    }

    // save all-time completed
    file = ti_Open("RanGen", "w");
    if (file) {
        ti_Write(&allTimeCompleted, sizeof(allTimeCompleted), 1, file);
        ti_Close(file);
    }

    free(behind_sprite);
    gfx_End();
    return 0;
}