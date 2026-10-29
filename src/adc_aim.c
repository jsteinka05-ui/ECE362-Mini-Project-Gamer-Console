#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#define JOYSTICK_X 45
#define JOYSTICK_Y 46
#define BUTTON_PIN 10

#define DEAD_LOW 1800
#define DEAD_HIGH 2300

// Example LCD dimensions - need to formulate the pixel size of the LCD
#define SCREEN_WIDTH
#define SCREEN_HEIGHT

// Latest ADC readings
volatile uint16_t joystick_x = 2048; //middle (0-4096)
volatile uint16_t joystick_y = 2048;  //middle (0-4096)

// Current aiming position
int aim_x = 120;
int aim_y = 250;

// Shooting variables
volatile bool shoot_requested = false;
int shot_x;
int shot_y;
int adc_channel = 5;

uint32_t last_press_time = 0; //helps with button debouncing





// Initialize joystick button
void init_button(void) {

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, button_callback);
}


// aiming cursor
void update_aim(void) {

    if (joystick_x < DEAD_LOW) {
        aim_x--;
    }
    else if (joystick_x > DEAD_HIGH) {
        aim_x++;
    }

    if (joystick_y < DEAD_LOW) {
        aim_y--;
    }
    else if (joystick_y > DEAD_HIGH) {
        aim_y++;
    }

    // Keep aim inside LCD boundaries
    if (aim_x < 0) {
        aim_x = 0;
    }
    if (aim_x >= SCREEN_WIDTH) {
        aim_x = SCREEN_WIDTH - 1;
    }

    if (aim_y < 0) {
        aim_y = 0;
    }
    if (aim_y >= SCREEN_HEIGHT) {
        aim_y = SCREEN_HEIGHT - 1;
    }
}


// Shoot toward current aim position - doesn't reset current x and y positions
void shoot_ball(void) {

    shot_x = aim_x;
    shot_y = aim_y;

    printf("SHOT! X: %d, Y: %d\n", shot_x, shot_y);

    // maybe might put something for teammates to add ball animation and scoring here

}


int main(void) {

    stdio_init_all();

    init_adc_freerun();
    init_button();

    while (true) {

        // Move cursor using latest ADC readings
        update_aim();

        // Shoot if button was pressed
        if (shoot_requested) {
            shoot_requested = false;
            shoot_ball();
        }

        sleep_ms(20);
    }
}