#ifndef CUP_PONG_DISPLAY_H
#define CUP_PONG_DISPLAY_H

// LCD size in landscape mode
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define TOTAL_CUPS 10

// Set up the table and cup positions
void cup_pong_display_init(void);

// Move the aiming crosshair
void cup_pong_display_aim(int x, int y);

// Animate a throw to the current target
// Returns 1 for a hit, 0 for a miss
int cup_pong_display_throw(void);

// Start a new round
void cup_pong_display_reset(void);

#endif
