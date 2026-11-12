#include "cup_pong_display.h"
#include "lcd.h"
#include "lcd_dma.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// RGB565 colors
#define TABLE_GREEN 0x1368
#define TABLE_LINE  0x8E76
#define CUP_RED     0xD145
#define CUP_DARK    0x7800
#define CUP_RIM     0xFFDF
#define CUP_INSIDE  0x5004
#define SHADOW      0x0843
#define METER_GRAY  0x3186
#define SWEET_GREEN 0x07E0

#define BALL_START_X 108
#define BALL_START_Y 281
#define BALL_RADIUS 7
#define ARC_HEIGHT 74
#define FLIGHT_FRAMES 30
#define PATCH_SIDE 48
#define METER_TOP 76
#define METER_BOTTOM 256

// Cached background and small DMA transfer buffer
static uint16_t background[SCREEN_WIDTH * SCREEN_HEIGHT];
static uint16_t patch[PATCH_SIDE * PATCH_SIDE];

// Cup positions: single front cup, then rows of 2, 3, and 4
static const int cup_x[TOTAL_CUPS] = {
    108, 88, 128, 69, 108, 147, 49, 88, 127, 166
};

static const int cup_y[TOTAL_CUPS] = {
    171, 140, 140, 110, 110, 110, 83, 83, 83, 83
};

static uint8_t cup_alive[TOTAL_CUPS];
static int cups_left = TOTAL_CUPS;
static int shots_taken = 0;

static int aim_x = 108;
static int aim_y = 171;
static int aim_visible = 1;
static int power_level = 50;

static int ball_x = BALL_START_X;
static int ball_y = BALL_START_Y;
static int ball_radius = BALL_RADIUS;
static int ball_visible = 1;

typedef struct {
    int x;
    int y;
    int r;
} ball_frame_t;

static int clamp_value(int value, int lo, int hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

static int cup_radius(int index) {
    if (index == 0) return 13;
    if (index <= 2) return 11;
    if (index <= 5) return 10;
    return 9;
}

static void bg_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    background[y * SCREEN_WIDTH + x] = color;
}

static void bg_span(int x1, int x2, int y, uint16_t color) {
    if (y < 0 || y >= SCREEN_HEIGHT) return;

    x1 = clamp_value(x1, 0, SCREEN_WIDTH - 1);
    x2 = clamp_value(x2, 0, SCREEN_WIDTH - 1);

    for (int x = x1; x <= x2; x++) {
        bg_pixel(x, y, color);
    }
}

static void bg_ellipse(int cx, int cy, int rx, int ry, uint16_t color) {
    if (rx <= 0 || ry <= 0) return;

    for (int dy = -ry; dy <= ry; dy++) {
        for (int dx = -rx; dx <= rx; dx++) {
            if (dx * dx * ry * ry + dy * dy * rx * rx <= rx * rx * ry * ry) {
                bg_pixel(cx + dx, cy + dy, color);
            }
        }
    }
}

// Draw a perspective cup onto the static background
static void draw_cup_into_background(int i) {
    if (!cup_alive[i]) return;

    int x = cup_x[i];
    int y = cup_y[i];
    int r = cup_radius(i);
    int half_height = r / 2;

    // Shadow
    bg_ellipse(x + 2, y + 19, r - 2, 4, SHADOW);

    // Tapered red cup body
    for (int row = 0; row <= 18; row++) {
        int side = r - (row * 3 / 18);
        bg_span(x - side, x + side, y + row, CUP_RED);
        bg_pixel(x + side, y + row, CUP_DARK);
        bg_pixel(x - side + 2, y + row, CUP_RIM);
    }

    // Elliptical top rim and dark cup opening
    bg_ellipse(x, y, r, half_height, CUP_RIM);
    bg_ellipse(x, y, r - 3, half_height - 2, CUP_INSIDE);
    bg_pixel(x - r / 2, y - half_height / 2, WHITE);
}

static void rebuild_background(void) {
    memset(background, 0, sizeof(background));

    // Perspective table: narrow at the top, wide at the bottom
    for (int y = 52; y <= 304; y++) {
        int left = 39 + (12 - 39) * (y - 52) / (304 - 52);
        int right = 176 + (196 - 176) * (y - 52) / (304 - 52);

        bg_span(left, right, y, TABLE_GREEN);

        if (y == 110 || y == 168 || y == 250) {
            bg_span(left, right, y, TABLE_LINE);
        }

        bg_pixel(left, y, WHITE);
        bg_pixel(right, y, WHITE);

        if (y >= 184 && y <= 301) {
            bg_pixel(108, y, TABLE_LINE);
        }
    }

    bg_span(39, 176, 52, WHITE);
    bg_span(12, 196, 304, WHITE);

    // Draw rear cups before front cups
    for (int i = TOTAL_CUPS - 1; i >= 0; i--) {
        draw_cup_into_background(i);
    }

    // Static power-meter track
    for (int y = METER_TOP; y <= METER_BOTTOM; y++) {
        bg_span(212, 227, y, METER_GRAY);

        uint16_t band = (y >= 157 && y <= 175) ? SWEET_GREEN : RED;
        bg_span(215, 224, y, band);

        bg_pixel(212, y, WHITE);
        bg_pixel(227, y, WHITE);
    }

    bg_span(212, 227, METER_TOP, WHITE);
    bg_span(212, 227, METER_BOTTOM, WHITE);
}

// Write a pixel into the small DMA patch buffer
static void tile_pixel(int x, int y, int x0, int y0, int width, int height, uint16_t color) {
    if (x < x0 || y < y0 || x >= x0 + width || y >= y0 + height) return;
    patch[(y - y0) * width + (x - x0)] = color;
}

static void tile_circle(int cx, int cy, int radius, int x0, int y0, int width, int height, uint16_t color) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                tile_pixel(cx + dx, cy + dy, x0, y0, width, height, color);
            }
        }
    }
}

// Paint moving items over the saved background
static void paint_overlays(int x0, int y0, int width, int height) {
    if (aim_visible) {
        for (int d = -7; d <= 7; d++) {
            tile_pixel(aim_x + d, aim_y, x0, y0, width, height, YELLOW);
            tile_pixel(aim_x, aim_y + d, x0, y0, width, height, YELLOW);
        }
    }

    // Current power-meter indicator
    int indicator_y = METER_BOTTOM - power_level * 180 / 100;

    for (int y = indicator_y - 2; y <= indicator_y + 2; y++) {
        for (int x = 207; x <= 233; x++) {
            tile_pixel(x, y, x0, y0, width, height, YELLOW);
        }
    }

    // Moving ball
    if (ball_visible) {
        tile_circle(ball_x + 1, ball_y + 2, ball_radius, x0, y0, width, height, SHADOW);
        tile_circle(ball_x, ball_y, ball_radius, x0, y0, width, height, WHITE);
        tile_circle(ball_x - 2, ball_y - 2, ball_radius / 3, x0, y0, width, height, TABLE_LINE);
    }
}

// Redraw only small changed rectangles using SPI DMA
static void present_rect(int x1, int y1, int x2, int y2) {
    x1 = clamp_value(x1, 0, SCREEN_WIDTH - 1);
    x2 = clamp_value(x2, 0, SCREEN_WIDTH - 1);
    y1 = clamp_value(y1, 0, SCREEN_HEIGHT - 1);
    y2 = clamp_value(y2, 0, SCREEN_HEIGHT - 1);

    if (x2 < x1 || y2 < y1) return;

    for (int top = y1; top <= y2; top += PATCH_SIDE) {
        for (int left = x1; left <= x2; left += PATCH_SIDE) {
            int width = x2 - left + 1;
            int height = y2 - top + 1;

            if (width > PATCH_SIDE) width = PATCH_SIDE;
            if (height > PATCH_SIDE) height = PATCH_SIDE;

            // Copy background pixels into contiguous transfer buffer
            for (int row = 0; row < height; row++) {
                memcpy(
                    &patch[row * width],
                    &background[(top + row) * SCREEN_WIDTH + left],
                    (size_t)width * sizeof(uint16_t)
                );
            }

            paint_overlays(left, top, width, height);

            lcd_dma_blit(left, top, width, height, patch);
        }
    }
}

// Erase the old ball and draw its new position
static void update_ball(int new_x, int new_y, int new_radius) {
    int x1 = ball_x - BALL_RADIUS - 4;
    int y1 = ball_y - BALL_RADIUS - 4;
    int x2 = ball_x + BALL_RADIUS + 4;
    int y2 = ball_y + BALL_RADIUS + 4;

    if (new_x - BALL_RADIUS - 4 < x1) x1 = new_x - BALL_RADIUS - 4;
    if (new_y - BALL_RADIUS - 4 < y1) y1 = new_y - BALL_RADIUS - 4;
    if (new_x + BALL_RADIUS + 4 > x2) x2 = new_x + BALL_RADIUS + 4;
    if (new_y + BALL_RADIUS + 4 > y2) y2 = new_y + BALL_RADIUS + 4;

    ball_x = new_x;
    ball_y = new_y;
    ball_radius = new_radius;
    ball_visible = 1;

    present_rect(x1, y1, x2, y2);
}

static void draw_score(void) {
    char text[22];

    LCD_DrawFillRectangle(0, 0, 239, 29, BLACK);

    snprintf(text, sizeof(text), "CUPS:%d", cups_left);
    LCD_DrawString(5, 7, WHITE, BLACK, text, 12, 0);

    snprintf(text, sizeof(text), "SHOTS:%d", shots_taken);
    LCD_DrawString(131, 7, WHITE, BLACK, text, 12, 0);
}

static void show_message(const char *text) {
    LCD_DrawFillRectangle(4, 30, 201, 49, BLACK);

    if (text != NULL) {
        LCD_DrawString(18, 33, YELLOW, BLACK, text, 12, 0);
    }
}

// Reset all game objects
void cup_pong_display_init(void) {
    for (int i = 0; i < TOTAL_CUPS; i++) {
        cup_alive[i] = 1;
    }

    cups_left = TOTAL_CUPS;
    shots_taken = 0;

    aim_x = 108;
    aim_y = 171;
    aim_visible = 1;
    power_level = 50;

    ball_x = BALL_START_X;
    ball_y = BALL_START_Y;
    ball_radius = BALL_RADIUS;
    ball_visible = 1;

    rebuild_background();

    // The only full-screen transfer during a round
    lcd_dma_blit(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, background);

    draw_score();
    show_message(NULL);
    LCD_DrawString(208, 48, WHITE, BLACK, "PWR", 12, 0);

    present_rect(aim_x - 9, aim_y - 9, aim_x + 9, aim_y + 9);
    present_rect(ball_x - 12, ball_y - 12, ball_x + 12, ball_y + 12);
    present_rect(205, 158, 235, 175);
}

// Move the crosshair without clearing the whole LCD
void cup_pong_display_aim(int x, int y) {
    int new_x = clamp_value(x, 35, 175);
    int new_y = clamp_value(y, 73, 176);

    if (new_x == aim_x && new_y == aim_y) return;

    int old_x = aim_x;
    int old_y = aim_y;

    aim_x = new_x;
    aim_y = new_y;

    int left = old_x < new_x ? old_x - 9 : new_x - 9;
    int top = old_y < new_y ? old_y - 9 : new_y - 9;
    int right = old_x > new_x ? old_x + 9 : new_x + 9;
    int bottom = old_y > new_y ? old_y + 9 : new_y + 9;

    present_rect(left, top, right, bottom);
}

// Update only the old and new power indicator positions
void cup_pong_display_set_power(int power) {
    int new_power = clamp_value(power, 0, 100);

    if (new_power == power_level) return;

    int old_y = METER_BOTTOM - power_level * 180 / 100;

    power_level = new_power;

    int new_y = METER_BOTTOM - power_level * 180 / 100;

    present_rect(205, old_y - 3, 235, old_y + 3);
    present_rect(205, new_y - 3, 235, new_y + 3);
}

// Precompute and animate a curved shot
int cup_pong_display_throw(void) {
    int landing_x = aim_x;
    int landing_y = aim_y;

    // Power controls whether ball falls short or overshoots
    if (power_level < 45) {
        landing_y += (45 - power_level) * 3;
    }

    if (power_level > 55) {
        landing_y -= (power_level - 55) * 3;
    }

    landing_y = clamp_value(landing_y, 60, 294);

    // Hide aim during the throw
    int old_aim_x = aim_x;
    int old_aim_y = aim_y;

    aim_visible = 0;
    present_rect(old_aim_x - 9, old_aim_y - 9, old_aim_x + 9, old_aim_y + 9);
    show_message(NULL);

    // Preset the entire parabolic flight before rendering
    ball_frame_t flight[FLIGHT_FRAMES + 1];

    for (int step = 0; step <= FLIGHT_FRAMES; step++) {
        int ground_x = BALL_START_X + (landing_x - BALL_START_X) * step / FLIGHT_FRAMES;
        int ground_y = BALL_START_Y + (landing_y - BALL_START_Y) * step / FLIGHT_FRAMES;

        int rise = 4 * ARC_HEIGHT * step * (FLIGHT_FRAMES - step) / (FLIGHT_FRAMES * FLIGHT_FRAMES);

        flight[step].x = ground_x;
        flight[step].y = ground_y - rise;
        flight[step].r = BALL_RADIUS - 3 * step / FLIGHT_FRAMES;
    }

    // Stream only the small changing ball region each frame
    for (int step = 1; step <= FLIGHT_FRAMES; step++) {
        update_ball(flight[step].x, flight[step].y, flight[step].r);
        sleep_ms(22);
    }

    shots_taken++;

    int hit_index = -1;

    // Hit test: power must be in the sweet spot
    if (power_level >= 45 && power_level <= 55) {
        for (int i = 0; i < TOTAL_CUPS; i++) {
            if (!cup_alive[i]) continue;

            int dx = landing_x - cup_x[i];
            int dy = landing_y - cup_y[i];
            int r = cup_radius(i) - 3;

            if (dx * dx + dy * dy <= r * r) {
                hit_index = i;
                break;
            }
        }
    }

    if (hit_index >= 0) {
        // Ball sinks into the cup
        for (int r = 3; r >= 1; r--) {
            update_ball(landing_x, landing_y + (4 - r), r);
            sleep_ms(65);
        }

        ball_visible = 0;
        cup_alive[hit_index] = 0;
        cups_left--;

        // Update cached scene, then erase only the hit cup
        rebuild_background();

        int r = cup_radius(hit_index) + 7;

        present_rect(
            cup_x[hit_index] - r,
            cup_y[hit_index] - r,
            cup_x[hit_index] + r,
            cup_y[hit_index] + 26
        );

        draw_score();

        if (cups_left == 0) {
            show_message("YOU WIN!");
        } else {
            show_message("NICE SHOT!");
        }
    } else {
        sleep_ms(120);

        // Missed ball disappears
        ball_visible = 0;
        present_rect(ball_x - 12, ball_y - 12, ball_x + 12, ball_y + 12);

        draw_score();

        if (power_level < 45) {
            show_message("TOO WEAK!");
        } else if (power_level > 55) {
            show_message("TOO STRONG!");
        } else {
            show_message("MISSED!");
        }
    }

    // Put a new ball at the bottom without clearing the LCD
    ball_x = BALL_START_X;
    ball_y = BALL_START_Y;
    ball_radius = BALL_RADIUS;
    ball_visible = 1;

    present_rect(ball_x - 12, ball_y - 12, ball_x + 12, ball_y + 12);

    sleep_ms(550);

    if (cups_left > 0) {
        show_message(NULL);

        aim_visible = 1;
        present_rect(aim_x - 9, aim_y - 9, aim_x + 9, aim_y + 9);
    }

    return hit_index >= 0;
}

int cup_pong_display_cups_left(void) {
    return cups_left;
}

void cup_pong_display_reset(void) {
    cup_pong_display_init();
}
