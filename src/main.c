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
#include "player.h"
#include "map.h"
#include "menu.h"

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
    generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock, showTestTiles);
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
        drawMap(map, 0);

        if (player.canMove) {
            selected = 0;
            playerMovement(&player);


            if (kb_IsDown(kb_KeyAlpha)) {
                generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock, showTestTiles);
                // mapSprite(*sprite, map);
                resetPlayer(&player, spawnX, spawnY);
                continue;
            }

            if (player.x > GFX_LCD_WIDTH) {
                generateMap(map, spawnX, spawnY, caveHeight, wsChance, blockVariety, spawnBlock, showTestTiles);
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

    snprintf(storedSettings.allTimeCompleted, sizeof(storedSettings.allTimeCompleted), "%d", allTimeCompleted);
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