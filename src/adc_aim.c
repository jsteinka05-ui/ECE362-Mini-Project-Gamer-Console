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

// Example LCD dimensions - need to formulate the pixel size of the LCD - add dimensions later
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 500

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
bool first_press = true;


/*  //Use if necessary
// Reset ADC sampling if the FIFO overflows
void reset_adc_sampling(void) {

    // Stop continuous conversions
    adc_run(false);

    // Wait for the current conversion to finish
    while (!(adc_hw->cs & ADC_CS_READY_BITS)) {
        tight_loop_contents();
    }

    // Empty old FIFO readings
    adc_fifo_drain();

    // Clear FIFO overflow flag (write 1 to clear)
    hw_clear_bits(&adc_hw->fcs, ADC_FCS_OVER_BITS);

    // Restart at ADC channel 5
    adc_select_input(5);
    adc_channel = 5;

    adc_run(true);
}
*/

// ADC FIFO interrupt handler
void adc_handler(void) {

    /*
    // Check if the FIFO overflowed
    if (adc_hw->fcs & ADC_FCS_OVER_BITS) {
        reset_adc_sampling();
        return;
    }
    */

    while (!adc_fifo_is_empty()) {

        uint16_t value = adc_fifo_get();

        if (!(value & 0x8000)) {  //checks that the error flag defined later in the code is set (bit 15)

            if (adc_channel == 5) {
                joystick_x = value & 0x0FFF;  //Extracts the lower 12 bits
            }
            else {
                joystick_y = value & 0x0FFF; //Extracts the lower 12 bits
            }
        }

        // Alternate expected channel
        if (adc_channel == 5) {
            adc_channel = 6;
        }
        else {
            adc_channel = 5;
        }
    }
}


// Continuous ADC sampling
void init_adc_freerun(void) {

    adc_init();

    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    adc_select_input(5);

    adc_set_round_robin((1u << 5) | (1u << 6)); //switches between the two channels

    //makes sure that the runing rate of the sample
    adc_set_clkdiv(47999.0f);

    adc_fifo_setup(true, false, 1, true, false);  //returns false for all DMA related events

    adc_fifo_drain();  //empties the FIFO

    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_handler);
    adc_irq_set_enabled(true);
    irq_set_enabled(ADC_IRQ_FIFO, true);

    adc_run(true);
}


// Joystick button interrupt
void button_callback(uint gpio, uint32_t events) {

    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_FALL)) {

        uint32_t time = time_us_32(); //datasheet had this

        // 150 ms debounce
        if ((uint32_t)(time - last_press_time) >= 150000) {
            shoot_requested = true;
            last_press_time = time;
        }
    }
}


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