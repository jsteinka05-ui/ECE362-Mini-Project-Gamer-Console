#include "pico/stdlib.h"
#include "lcd.h"
#include "lcd_spi.h"
#include "cup_pong_display.h"

int main(void) {
    stdio_init_all();

    // Initialize the LCD
    lcd_spi_init();
    LCD_Setup();

    // Initialize the Cup Pong game display
    cup_pong_display_init();

    // Temporary automatic demonstration
    const int test_shots[][2] = {
        {40, 170},
        {160, 53},
        {146, 80},
        {174, 80},
        {132, 107},
        {160, 107},
        {188, 107},
        {118, 134},
        {146, 134},
        {174, 134},
        {202, 134}
    };

    while (1) {
        for (int i = 0; i < 11; i++) {
            int x = test_shots[i][0];
            int y = test_shots[i][1];

            // Move the aiming cursor
            cup_pong_display_aim(x, y);
            sleep_ms(400);

            // Throw the ball
            cup_pong_display_throw();
            sleep_ms(600);
        }

        // Display the win screen before restarting
        sleep_ms(2500);
        cup_pong_display_reset();
    }
}
