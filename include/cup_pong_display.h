
#ifndef CUP_PONG_DISPLAY_H
#define CUP_PONG_DISPLAY_H

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define TOTAL_CUPS 10

void cup_pong_display_init(void);
void cup_pong_display_aim(int x, int y);
void cup_pong_display_set_power(int power);
int cup_pong_display_throw(void);
int cup_pong_display_cups_left(void);
void cup_pong_display_reset(void);

#endif
