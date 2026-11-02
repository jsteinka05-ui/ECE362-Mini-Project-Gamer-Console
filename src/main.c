#include "pico/stdlib.h"
#include "lcd.h"
#include "lcd_spi.h"
#include "cup_pong_display.h"

// 1 = LCD color test; 0 = automatic Cup Pong demo
#define LCD_COLOR_TEST 1

int main(void) {
    stdio_init_all();

    // Initialize SPI0 and the LCD
    lcd_spi_init();
    LCD_Setup();

#if LCD_COLOR_TEST

    // Test LCD colors
    while (1) {
        LCD_Clear(RED);
        sleep_ms(1000);

        LCD_Clear(GREEN);
        sleep_ms(1000);

        LCD_Clear(BLUE);
        sleep_ms(1000);

        LCD_Clear(WHITE);
        sleep_ms(1000);

        LCD_Clear(BLACK);
        sleep_ms(1000);
    }

#else

    // Initialize Cup Pong
    cup_pong_display_init();

    // First shot misses, remaining shots target each cup
    const int test_shots[][2] = {
        {40, 215},
        {120, 132},
        {106, 105},
        {134, 105},
        {92, 78},
        {120, 78},
        {148, 78},
        {78, 51},
        {106, 51},
        {134, 51},
        {162, 51}
    };

    while (1) {
        for (int i = 0; i < 11; i++) {
            int x = test_shots[i][0];
            int y = test_shots[i][1];

            // Move aiming cursor
            cup_pong_display_aim(x, y);
            sleep_ms(400);

            // Throw the ball
            cup_pong_display_throw();
            sleep_ms(600);
        }

        // Show winning screen before restarting
        sleep_ms(2500);
        cup_pong_display_reset();
    }

#endif
}
