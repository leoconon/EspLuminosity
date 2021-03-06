#ifndef DRAWING
#define DRAWING

#include <libs.h>

#define MAX_WIDTH 128
#define MAX_HEIGHT 64
#define LUMINOSITY_MAX_VALUE 1000
#define STAR_CHAR_HEX 0x2605
#define HISTORY_MAX_HEIGHT 20
#define DRAW_COLOR_BLACK 0
#define DRAW_COLOR_WHITE 1

void drawHistoric(u8g2_t *u8g2, int newValue);
void drawMoon(u8g2_t *u8g2);
void drawSun(u8g2_t *u8g2);
void animateSun(u8g2_t *u8g2, bool small);
void clearDraw(u8g2_t *u8g2);
void printValue(u8g2_t *u8g2, int value);

#endif