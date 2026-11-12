#include "pico/stdlib.h"
#include "lcd.h"
#include "lcd_spi.h"
#include "lcd_dma.h"
#include "cup_pong_display.h"
#include "adc_aim.h"
#include "sound.h"

// 1 = LCD color test, 0 = Cup Pong
#define LCD_COLOR_TEST 0

// 1 = automatic demonstration without joystick
// 0 = actual playable game
#define AUTO_DEMO 0

#if !LCD_COLOR_TEST && AUTO_DEMO
static void auto_shot(int x, int y, int power) {
    cup_pong_display_aim(x, y);
    cup_pong_display_set_power(power);

    sleep_ms(400);

    sound_play(SFX_THROW);

    int hit = cup_pong_display_throw();

    if (cup_pong_display_cups_left() == 0) {
        sound_play(SFX_WIN);
    } else {
        sound_play(hit ? SFX_CUP_HIT : SFX_MISS);
    }

    sleep_ms(350);
}
#endif

int main(void) {
    stdio_init_all();

    lcd_spi_init();
    LCD_Setup();

#if LCD_COLOR_TEST

    // Verify LCD hardware before playing
    while (true) {
        LCD_Clear(RED);
        sleep_ms(700);

        LCD_Clear(GREEN);
        sleep_ms(700);

        LCD_Clear(BLUE);
        sleep_ms(700);

        LCD_Clear(BLACK);
        sleep_ms(700);
    }

#else

    lcd_dma_init();

    sound_init();
    sound_set_volume(50);

#if AUTO_DEMO

    const int cup_x[TOTAL_CUPS] = {
        108, 88, 128, 69, 108, 147, 49, 88, 127, 166
    };

    const int cup_y[TOTAL_CUPS] = {
        171, 140, 140, 110, 110, 110, 83, 83, 83, 83
    };

    while (true) {
        cup_pong_display_init();
        sleep_ms(900);

        // Weak shot
        auto_shot(108, 171, 25);

        // Strong shot
        auto_shot(108, 171, 75);

        // Hit all ten cups
        for (int i = 0; i < TOTAL_CUPS; i++) {
            auto_shot(cup_x[i], cup_y[i], 50);
        }

        sleep_ms(2000);
    }

#else

    adc_aim_init();
    cup_pong_display_init();

    sound_play(SFX_MENU_SELECT);

    int aim_x = 108;
    int aim_y = 171;

    int power = 0;
    int direction = 1;

    while (true) {
        // When all cups are gone, button starts a new game
        if (cup_pong_display_cups_left() == 0) {
            if (adc_aim_button_pressed()) {
                aim_x = 108;
                aim_y = 171;
                power = 0;
                direction = 1;

                cup_pong_display_reset();
                cup_pong_display_set_power(power);

                sound_play(SFX_MENU_SELECT);
            }

            sleep_ms(25);
            continue;
        }

        // Joystick controls aiming
        adc_aim_update(&aim_x, &aim_y);
        cup_pong_display_aim(aim_x, aim_y);

        // Power meter moves automatically
        power += 2 * direction;

        if (power >= 100) {
            power = 100;
            direction = -1;
        }

        if (power <= 0) {
            power = 0;
            direction = 1;
        }

        cup_pong_display_set_power(power);

        // Button locks power and launches ball
        if (adc_aim_button_pressed()) {
            sound_play(SFX_THROW);

            int hit = cup_pong_display_throw();

            if (cup_pong_display_cups_left() == 0) {
                sound_play(SFX_WIN);
            } else {
                sound_play(hit ? SFX_CUP_HIT : SFX_MISS);
            }

            power = 0;
            direction = 1;
            cup_pong_display_set_power(power);
        }

        sleep_ms(25);
    }

#endif
#endif
}
