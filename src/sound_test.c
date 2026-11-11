/* Standalone test for the PWM sound effects.
 * Built only by the [env:sound_test] PlatformIO environment, so it never
 * clashes with main.c / adc_aim.c (each has its own main()).
 *
 * Interrupt sources that trigger sounds here:
 *   - Button A (GPIO20) falling edge -> play the NEXT sound in the list
 *   - Button B (GPIO17) falling edge -> TOGGLE the auto-demo on/off
 *   - 2 s repeating timer            -> plays the next sound automatically
 *                                       (only while the auto-demo is on)
 * All of them just call sound_play(), exactly what game code will do later.
 *
 * Default demo state comes from AUTO_DEMO (1 = on at boot, 0 = silent at boot).
 * Override in platformio.ini with build_flags = -DAUTO_DEMO=0 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "sound.h"

#define BTN_A 20
#define BTN_B 17

#ifndef AUTO_DEMO
#define AUTO_DEMO 1
#endif

static const char *names[SFX_COUNT] = {
    "MENU_MOVE", "MENU_SELECT", "THROW", "CUP_HIT", "MISS", "WIN", "HIGH_SCORE"
};

static volatile int      last_id = 0;
static volatile int      next_id = 0;
static volatile uint32_t play_count = 0;
static volatile bool     demo_enabled = AUTO_DEMO;
static volatile uint32_t toggle_count = 0;
static uint32_t last_a_us, last_b_us;

static void play_next(void)
{
    last_id = next_id;
    next_id = (next_id + 1) % SFX_COUNT;
    play_count++;
    sound_play((sfx_id_t)last_id);
}

/* Only ONE gpio callback can be registered per core, so this lives in the
 * test only. In the real game, call sound_play() from your existing handlers. */
static void gpio_cb(uint gpio, uint32_t events)
{
    (void)events;
    uint32_t now = time_us_32();
    if (gpio == BTN_A && (uint32_t)(now - last_a_us) >= 150000) {
        last_a_us = now;
        play_next();
    } else if (gpio == BTN_B && (uint32_t)(now - last_b_us) >= 150000) {
        last_b_us = now;
        demo_enabled = !demo_enabled;
        toggle_count++;
        if (!demo_enabled) sound_stop();          /* cut off anything playing */
    }
}

static bool demo_timer_cb(repeating_timer_t *t)
{
    (void)t;
    if (demo_enabled) play_next();                /* sound_play is ISR-safe */
    return true;
}

int main(void)
{
    stdio_init_all();
    sound_init();

    const uint pins[] = { BTN_A, BTN_B };
    for (int i = 0; i < 2; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);                    /* buttons switch to GND */
    }
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, gpio_cb);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);

    static repeating_timer_t demo_timer;
    add_repeating_timer_ms(2000, demo_timer_cb, NULL, &demo_timer);

    uint32_t reported_plays = 0, reported_toggles = 0;
    while (true) {
        if (toggle_count != reported_toggles) {   /* print outside the ISR */
            reported_toggles = toggle_count;
            printf("Auto-demo %s\n", demo_enabled ? "ON" : "OFF");
        }
        if (play_count != reported_plays) {
            reported_plays = play_count;
            printf("Playing sfx %d: %s\n", last_id, names[last_id]);
        }
        sleep_ms(10);
    }
}