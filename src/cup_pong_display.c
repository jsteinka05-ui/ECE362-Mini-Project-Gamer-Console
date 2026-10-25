#include "cup_pong_display.h"
#include "lcd.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Table and cup colors
#define TABLE_COLOR 0x12C9
#define CUP_COLOR RED
#define CUP_INSIDE 0x8000
#define AIM_COLOR YELLOW
#define BALL_COLOR WHITE

#define CUP_RADIUS 10
#define BALL_RADIUS 4

// Cup locations on the screen
static const int cup_x[TOTAL_CUPS] = {
    160,
    146, 174,
    132, 160, 188,
    118, 146, 174, 202
};

static const int cup_y[TOTAL_CUPS] = {
    53,
    80, 80,
    107, 107, 107,
    134, 134, 134, 134
};

// 1 means the cup is still on the table
static int cups[TOTAL_CUPS];

static int target_x = 160;
static int target_y = 100;

static int cups_left = TOTAL_CUPS;
static int shots_taken = 0;

// Keep values within a range
static int clamp_value(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Draw one cup
static void draw_cup(int i) {
    if (!cups[i]) {
        return;
    }

    LCD_Circle(cup_x[i], cup_y[i], CUP_RADIUS, 1, CUP_COLOR);
    LCD_Circle(cup_x[i], cup_y[i], 6, 1, CUP_INSIDE);
}

// Draw all remaining cups
static void draw_cups(void) {
    for (int i = 0; i < TOTAL_CUPS; i++) {
        draw_cup(i);
    }
}

// Draw the score at the top
static void draw_score(void) {
    char text[20];

    LCD_DrawFillRectangle(0, 0, 319, 32, BLACK);

    sprintf(text, "CUPS: %d", cups_left);
    LCD_DrawString(10, 9, WHITE, BLACK, text, 16, 0);

    sprintf(text, "SHOTS: %d", shots_taken);
    LCD_DrawString(205, 9, WHITE, BLACK, text, 16, 0);
}

// Redraw a small area of the table
// This lets us erase moving objects without clearing the full LCD
static void restore_area(int x, int y, int radius) {
    int x1 = clamp_value(x - radius, 5, 314);
    int y1 = clamp_value(y - radius, 36, 230);
    int x2 = clamp_value(x + radius, 5, 314);
    int y2 = clamp_value(y + radius, 36, 230);

    LCD_DrawFillRectangle(x1, y1, x2, y2, TABLE_COLOR);

    // Redraw cups that overlap the cleared area
    for (int i = 0; i < TOTAL_CUPS; i++) {
        if (!cups[i]) {
            continue;
        }

        if (cup_x[i] + CUP_RADIUS >= x1 &&
            cup_x[i] - CUP_RADIUS <= x2 &&
            cup_y[i] + CUP_RADIUS >= y1 &&
            cup_y[i] - CUP_RADIUS <= y2) {
            draw_cup(i);
        }
    }
}

// Draw the aiming crosshair
static void draw_crosshair(int x, int y) {
    LCD_DrawFillRectangle(x - 1, y - 13, x + 1, y - 6, AIM_COLOR);
    LCD_DrawFillRectangle(x - 1, y + 6, x + 1, y + 13, AIM_COLOR);
    LCD_DrawFillRectangle(x - 13, y - 1, x - 6, y + 1, AIM_COLOR);
    LCD_DrawFillRectangle(x + 6, y - 1, x + 13, y + 1, AIM_COLOR);
}

// Draw the table and initial cups
void cup_pong_display_init(void) {
    LCD_Clear(BLACK);

    // Green table
    LCD_DrawFillRectangle(5, 35, 314, 231, TABLE_COLOR);

    // Table outline
    LCD_DrawRectangle(4, 34, 315, 232, WHITE);

    // Reset cup states
    for (int i = 0; i < TOTAL_CUPS; i++) {
        cups[i] = 1;
    }

    cups_left = TOTAL_CUPS;
    shots_taken = 0;

    target_x = 160;
    target_y = 100;

    draw_score();
    draw_cups();

    // Ball starting position
    LCD_Circle(160, 210, BALL_RADIUS, 1, BALL_COLOR);

    draw_crosshair(target_x, target_y);
}

// Move the aiming crosshair
void cup_pong_display_aim(int x, int y) {
    x = clamp_value(x, 20, 300);
    y = clamp_value(y, 50, 175);

    if (x == target_x && y == target_y) {
        return;
    }

    // Erase the old crosshair
    restore_area(target_x, target_y, 14);

    target_x = x;
    target_y = y;

    // Draw it at the new position
    draw_crosshair(target_x, target_y);
}

// Animate the ball and check whether it hits a cup
int cup_pong_display_throw(void) {
    int start_x = 160;
    int start_y = 210;

    int old_x = start_x;
    int old_y = start_y;

    // Hide the crosshair during the throw
    restore_area(target_x, target_y, 14);

    // Animate ball traveling toward the target
    for (int step = 1; step <= 20; step++) {
        int x = start_x + (target_x - start_x) * step / 20;
        int y = start_y + (target_y - start_y) * step / 20;

        restore_area(old_x, old_y, BALL_RADIUS + 1);
        LCD_Circle(x, y, BALL_RADIUS, 1, BALL_COLOR);

        old_x = x;
        old_y = y;

        sleep_ms(20);
    }

    sleep_ms(100);

    restore_area(old_x, old_y, BALL_RADIUS + 1);

    shots_taken++;

    int hit = 0;

    // Check whether the target is inside an active cup
    for (int i = 0; i < TOTAL_CUPS; i++) {
        if (!cups[i]) {
            continue;
        }

        int dx = target_x - cup_x[i];
        int dy = target_y - cup_y[i];

        if (dx * dx + dy * dy <= 8 * 8) {
            cups[i] = 0;
            cups_left--;
            hit = 1;

            // Remove the hit cup
            restore_area(cup_x[i], cup_y[i], CUP_RADIUS + 1);
            break;
        }
    }

    draw_score();

    // Put the ball back at the start
    LCD_Circle(start_x, start_y, BALL_RADIUS, 1, BALL_COLOR);

    if (cups_left == 0) {
        LCD_DrawFillRectangle(75, 165, 245, 193, BLACK);
        LCD_DrawString(105, 171, GREEN, BLACK, "YOU WIN!", 16, 0);
    } else {
        draw_crosshair(target_x, target_y);
    }

    return hit;
}

// Start a new round
void cup_pong_display_reset(void) {
    cup_pong_display_init();
}
