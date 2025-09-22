#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>
#include "globals.h"
#include "appvar_utils.h"

struct Menu {
    bool show;
    char* title;

    int infoLen;
    char** infoList;

    int showOpts; // display the first x options, this wont disable any though
    int optLen;
    char** optList;
};

struct MenuData {
    int spawnX;
    int spawnY;
    int spawnBlock;
    int caveHeight;
    int wsChance;
    int blockVariety;
    int showTestTiles;
};

int adjustParam(int param, int min, int max) {
    int adj = (int)kb_IsDown(kb_KeyRight)/4 - (int)kb_IsDown(kb_KeyLeft)/2;
    if (kb_IsDown(kb_KeyLeft) || kb_IsDown(kb_KeyRight)) dbg_printf("adjusting by %d\n", adj);
    int new = param + adj;

    if (new < min || new > max) return param;
    else return new;
}

const char* handleMenuMode(struct Menu *menu, struct settings *storedSettings, struct MenuData *data, const char *menuMode, int selected) {
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

            sprintf(storedSettings->spawnX, "%d", data->spawnX);
            sprintf(storedSettings->spawnY, "%d", data->spawnY);
            sprintf(storedSettings->spawnBlock, "%d", data->spawnBlock);
            sprintf(storedSettings->caveHeight, "%d", data->caveHeight);
            sprintf(storedSettings->wsChance, "%d", data->wsChance);
            sprintf(storedSettings->blockVariety, "%d", data->blockVariety);
            sprintf(storedSettings->showTestTiles, "%d", data->showTestTiles);
            
            bool passed[7] = {true, true, true, true, true, true, true};
            if (!writeKeyValue("RanGen", "spawnX", storedSettings->spawnX))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "spawnY", storedSettings->spawnY))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "spawnBlock", storedSettings->spawnBlock))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "caveHeight", storedSettings->caveHeight))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "wsChance", storedSettings->wsChance))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "blockVariety", storedSettings->blockVariety))
                {dbg_printf("failed to write spawnX\n"); passed[0] = false;}
            if (!writeKeyValue("RanGen", "showTestTiles", storedSettings->showTestTiles))
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
            data->spawnX = DEFAULT_SPAWNX;
            data->spawnY = DEFAULT_SPAWNY;
            data->spawnBlock = DEFAULT_SPAWN_BLOCK;
            data->caveHeight = DEFAULT_CAVE_HEIGHT;
            data->wsChance = DEFAULT_WS_CHANCE;
            data->blockVariety = DEFAULT_BLOCK_VARIETY;
            data->showTestTiles = DEFAULT_SHOW_TEST_TILES;
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

int drawMenu(struct Menu *menu, struct MenuData *data, const char *mode, int selected) {
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
        drawSlider(160, 78, data->spawnX, 3, 0, GFX_LCD_WIDTH, (selected == 3));
        gfx_PrintString(", ");
        drawSlider(gfx_GetTextX(), 78, data->spawnY, 3, 0, GFX_LCD_HEIGHT, (selected == 4));
        drawSlider(160, gfx_GetTextY() + 10, data->spawnBlock, 1, 0, 1, (selected == 5));
        drawSlider(160, gfx_GetTextY() + 10, data->caveHeight, 3, -99, 999, (selected == 6));
        drawSlider(160, gfx_GetTextY() + 10, data->wsChance, 3, 0, 100, (selected == 7));
        drawSlider(160, gfx_GetTextY() + 10, data->blockVariety, 3, 0, 100, (selected == 8));
        drawSlider(160, gfx_GetTextY() + 10, data->showTestTiles, 1, 0, 0, (selected == 9));
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
