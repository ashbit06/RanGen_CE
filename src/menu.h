#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>

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

int adjustParam(int param, int min, int max);
const char* handleMenuMode(struct Menu *menu, struct MenuData *data, const char *menuMode, int selected);
void drawSlider(int x, int y, int value, int padding, int min, int max, bool showArrows);
int drawMenu(struct Menu *menu, struct MenuData *data, const char *mode, int selected);

#endif // MENU_H