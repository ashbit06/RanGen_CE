#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include <stdlib.h>
#include <math.h>

#define TILE_SIZE 16

struct Player {
    int x;
    int y;
    int dx;
    int dy;
};

void drawPlayer(int x,int y) {
    gfx_SetColor(gfx_red);
    gfx_FillRectangle(x, y, (int)(TILE_SIZE/2), (int)(TILE_SIZE/2));
}

void drawTile(int x, int y, int type) {
    switch (type) {
        case 0:
        default:
            gfx_SetColor(gfx_black);
            gfx_FillRectangle(x, y, TILE_SIZE, TILE_SIZE);
    }
}

void generateMap(int spawnX, int spawnY, int caveHeight, int wsChance) {
    for (int y = 0; y < GFX_LCD_HEIGHT; y+=TILE_SIZE) {
        for (int x = 0; x < GFX_LCD_WIDTH; x+=TILE_SIZE) {
            if ((((int)rand() * ((int)log(y) * 170 + 170 + abs(caveHeight-500))) + 1) > (wsChance * 6) || ((x == spawnX) && (y == spawnY - TILE_SIZE)))
                drawTile(x, y, 0);
        }
    }
}
// tiles to fit: 24x18

int main() {
    // Initialize the graphics
    gfx_Begin();
    gfx_SetDrawBuffer();

    // initialize font
    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(gfx_green);

    // initialize random
    srand(rtc_Time());

    // initialize the player
    struct Player p;
    p.x = 10;
    p.y = (int)(GFX_LCD_HEIGHT / 2);

    int spawnX = GFX_LCD_WIDTH/2 + TILE_SIZE/2;
    int spawnY = GFX_LCD_HEIGHT/2 + TILE_SIZE/2;
    int caveHeight = 0;
    int wsChance = 60;


    // Main game loop
    while (1) {
        kb_Scan();  // Scan the keypad

        if (kb_IsDown(kb_KeyClear)) { // Exit if Clear is pressed
            break;
        }

        gfx_FillScreen(gfx_white);

        // generate map
        generateMap(spawnX, spawnY, caveHeight, wsChance);

        // player controls
        if (kb_IsDown(kb_KeyRight)) {
            p.x += 2;
        } else if (kb_IsDown(kb_KeyLeft)) {
            p.x -= 2;
        }
        drawPlayer(p.x, p.y);

        gfx_PrintInt(p.x, 0);
        gfx_PrintInt(p.y, 0);

        // Swap buffers to show the updated screen
        gfx_SwapDraw();

        // Delay for a short period
        delay(50);
    }

    // Close the graphics and clean up
    gfx_End();
    return 0;
}
