#include "adc_aim.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

// Joey's PCB: GP40/ADC0, GP41/ADC1, button GP10
#define JOYSTICK_X 40
#define JOYSTICK_Y 41
#define ADC_X_CHANNEL 0
#define ADC_Y_CHANNEL 1
#define BUTTON_PIN 10

#define DEAD_LOW 1750
#define DEAD_HIGH 2350

static bool last_down = false;
static uint32_t last_press_us = 0;

static int clamp_value(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void adc_aim_init(void) {
    adc_init();

    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    last_down = !gpio_get(BUTTON_PIN);
    last_press_us = time_us_32() - 200000;
}

void adc_aim_update(int *x, int *y) {
    adc_select_input(ADC_X_CHANNEL);
    uint16_t raw_x = adc_read();

    adc_select_input(ADC_Y_CHANNEL);
    uint16_t raw_y = adc_read();

    int dx = 0;
    int dy = 0;

    // Faster movement near the ends of joystick travel
    if (raw_x < DEAD_LOW) dx = raw_x < 700 ? -2 : -1;
    if (raw_x > DEAD_HIGH) dx = raw_x > 3400 ? 2 : 1;

    if (raw_y < DEAD_LOW) dy = raw_y < 700 ? -2 : -1;
    if (raw_y > DEAD_HIGH) dy = raw_y > 3400 ? 2 : 1;

    *x = clamp_value(*x + dx, 35, 175);
    *y = clamp_value(*y + dy, 73, 176);
}

bool adc_aim_button_pressed(void) {
    bool down = !gpio_get(BUTTON_PIN);
    bool edge = down && !last_down;

    last_down = down;

    if (!edge) return false;

    uint32_t now = time_us_32();

    if ((uint32_t)(now - last_press_us) < 170000) {
        return false;
    }

    last_press_us = now;
    return true;
}
