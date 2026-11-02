
#include "cup_pong_display.h"
#include "lcd.h"
#include "pico/stdlib.h"
#include <stdio.h>

// RGB565 colors and object dimensions
#define TABLE_COLOR 0x12C9
#define CUP_COLOR RED
#define CUP_INSIDE 0x8000
#define AIM_COLOR YELLOW
#define BALL_COLOR WHITE
#define LINE_COLOR 0xBDF7
#define CUP_RADIUS 10
#define BALL_RADIUS 4

// 4-3-2-1 arrangement: tip (index 0) is closest to the player
static const int cup_x[TOTAL_CUPS] = {
    120,
    106, 134,
    92, 120, 148,
    78, 106, 134, 162
};

static const int cup_y[TOTAL_CUPS] = {
    132,
    105, 105,
    78, 78, 78,
    51, 51, 51, 51
};

static int cups[TOTAL_CUPS];
static int target_x = 120;
static int target_y = 170;
static int cups_left = TOTAL_CUPS;
static int shots_taken = 0;

static int clamp_value(int value, int lo, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

static void draw_cup(int i) {
    if (!cups[i]) return;

    LCD_Circle(cup_x[i], cup_y[i], CUP_RADIUS, 1, CUP_COLOR);
    LCD_Circle(cup_x[i], cup_y[i], 6, 1, CUP_INSIDE);
}

static void draw_cups(void) {
    for (int i = 0; i < TOTAL_CUPS; i++) {
        draw_cup(i);
    }
}

static void draw_score(void) {
    char text[22];

    LCD_DrawFillRectangle(0, 0, SCREEN_WIDTH - 1, 24, BLACK);

    snprintf(text, sizeof(text), "CUPS:%d", cups_left);
    LCD_DrawString(6, 5, WHITE, BLACK, text, 12, 0);

    snprintf(text, sizeof(text), "SHOTS:%d", shots_taken);
    LCD_DrawString(149, 5, WHITE, BLACK, text, 12, 0);
}

// Clear only the affected region and restore table markings and cups
static void restore_area(int x, int y, int radius) {
    int x1 = clamp_value(x - radius, 12, 227);
    int y1 = clamp_value(y - radius, 31, 302);
    int x2 = clamp_value(x + radius, 12, 227);
    int y2 = clamp_value(y + radius, 31, 302);

    LCD_DrawFillRectangle(x1, y1, x2, y2, TABLE_COLOR);

    // Restore center line behind cups
    if (x1 <= 120 && x2 >= 120 && y2 >= 145) {
        int ly1 = y1 > 145 ? y1 : 145;
        LCD_DrawFillRectangle(120, ly1, 120, y2, LINE_COLOR);
    }

    // Redraw cups that overlap the cleared area
    for (int i = 0; i < TOTAL_CUPS; i++) {
        if (!cups[i]) continue;

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

// Initialize the Cup Pong screen
void cup_pong_display_init(void) {
    LCD_Clear(BLACK);

    // Draw the green table
    LCD_DrawFillRectangle(12, 30, 227, 303, TABLE_COLOR);
    LCD_DrawRectangle(11, 29, 228, 304, WHITE);

    // Draw the center line
    LCD_DrawFillRectangle(120, 145, 120, 302, LINE_COLOR);

    // Reset all cups
    for (int i = 0; i < TOTAL_CUPS; i++) {
        cups[i] = 1;
    }

    cups_left = TOTAL_CUPS;
    shots_taken = 0;
    target_x = 120;
    target_y = 170;

    draw_score();
    draw_cups();

    // Starting ball position
    LCD_Circle(120, 282, BALL_RADIUS, 1, BALL_COLOR);

    draw_crosshair(target_x, target_y);
}

// Move the aiming cursor
void cup_pong_display_aim(int x, int y) {
    x = clamp_value(x, 27, 212);
    y = clamp_value(y, 45, 255);

    if (x == target_x && y == target_y) {
        return;
    }

    // Remove previous crosshair
    restore_area(target_x, target_y, 14);

    target_x = x;
    target_y = y;

    // Draw new crosshair
    draw_crosshair(target_x, target_y);
}

// Animate a ball throw and check for a cup hit
int cup_pong_display_throw(void) {
    const int start_x = 120;
    const int start_y = 282;

    int old_x = start_x;
    int old_y = start_y;

    // Hide crosshair while throwing
    restore_area(target_x, target_y, 14);

    // Move ball toward selected target
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

    // Remove animated ball
    restore_area(old_x, old_y, BALL_RADIUS + 1);

    shots_taken++;

    int hit = 0;

    // Determine whether the ball landed inside an active cup
    for (int i = 0; i < TOTAL_CUPS; i++) {
        if (!cups[i]) continue;

        int dx = target_x - cup_x[i];
        int dy = target_y - cup_y[i];

        if (dx * dx + dy * dy <= 8 * 8) {
            cups[i] = 0;
            cups_left--;
            hit = 1;

            // Remove the cup from the display
            restore_area(cup_x[i], cup_y[i], CUP_RADIUS + 1);
            break;
        }
    }

    draw_score();

    // Restore ball to the bottom of the screen
    LCD_Circle(start_x, start_y, BALL_RADIUS, 1, BALL_COLOR);

    if (cups_left == 0) {
        LCD_DrawFillRectangle(60, 172, 184, 199, BLACK);
        LCD_DrawString(86, 180, GREEN, BLACK, "YOU WIN!", 12, 0);
    } else {
        draw_crosshair(target_x, target_y);
    }

    return hit;
}

// Reset game
void cup_pong_display_reset(void) {
    cup_pong_display_init();
}
