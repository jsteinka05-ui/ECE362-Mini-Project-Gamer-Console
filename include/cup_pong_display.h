
#ifndef CUP_PONG_DISPLAY_H
#define CUP_PONG_DISPLAY_H

// Portrait orientation: 240 x 320 pixels
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define TOTAL_CUPS 10

// Draw table and reset all cups, shots and aim
void cup_pong_display_init(void);

// Move the aiming crosshair to a screen coordinate
void cup_pong_display_aim(int x, int y);

// Animate a throw. Returns 1 on a hit, 0 on a miss.
int cup_pong_display_throw(void);

// Reset and redraw the game
void cup_pong_display_reset(void);

#endif
