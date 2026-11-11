#include "sound.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"

/* ---------------------------------------------------------------------------
 * A sound effect is a list of notes {frequency Hz, duration ms}, ended by
 * NOTE_END. freq = 0 is a rest. A 1 ms repeating-timer interrupt walks the
 * list, so playback never blocks the game loop.
 *
 * Hardware note: R3 (330k) + C3 (10nF) + the 10k pot form a low-pass with a
 * corner near 1.6 kHz, so keep effects mostly under ~1.5 kHz for full volume.
 * ------------------------------------------------------------------------- */
typedef struct { uint16_t freq; uint16_t ms; } note_t;
#define NOTE_END {0, 0}
#define REST 0

#define G3 196
#define C4 262
#define E4 330
#define G4 392
#define C5 523
#define E5 659
#define G5 784
#define C6 1047

static const note_t sfx_menu_move[]   = { {660, 25}, NOTE_END };
static const note_t sfx_menu_select[] = { {C5, 50}, {G5, 80}, NOTE_END };
static const note_t sfx_throw[]       = { {300, 30}, {400, 30}, {500, 30}, {600, 30}, {700, 30}, NOTE_END };
static const note_t sfx_cup_hit[]     = { {C5, 70}, {E5, 70}, {G5, 140}, NOTE_END };
static const note_t sfx_miss[]        = { {E4, 100}, {C4, 220}, NOTE_END };
static const note_t sfx_win[]         = { {C5, 100}, {E5, 100}, {G5, 100}, {C6, 250}, {REST, 50},
                                          {G5, 100}, {C6, 350}, NOTE_END };
static const note_t sfx_high_score[]  = { {G4, 90}, {C5, 90}, {E5, 90}, {G5, 90}, {REST, 40},
                                          {E5, 90}, {G5, 90}, {C6, 300}, NOTE_END };

typedef struct {
    const note_t *notes;
    uint8_t priority;       /* higher = harder to interrupt */
} sfx_def_t;

/* Indexed by sfx_id_t */
static const sfx_def_t sfx_table[SFX_COUNT] = {
    [SFX_MENU_MOVE]   = { sfx_menu_move,   0 },
    [SFX_MENU_SELECT] = { sfx_menu_select, 1 },
    [SFX_THROW]       = { sfx_throw,       1 },
    [SFX_CUP_HIT]     = { sfx_cup_hit,     2 },
    [SFX_MISS]        = { sfx_miss,        1 },
    [SFX_WIN]         = { sfx_win,         3 },
    [SFX_HIGH_SCORE]  = { sfx_high_score,  3 },
};

/* ---- State shared between main code and the timer ISR ---- */
static uint slice_num;
static uint8_t volume = 50;                       /* 0-100 */
static repeating_timer_t tick_timer;

static const note_t * volatile cur_notes = NULL;
static volatile uint8_t  cur_idx = 0;
static volatile uint16_t ms_left = 0;
static volatile uint8_t  cur_priority = 0;
static volatile bool     playing = false;

/* Output a square wave at 'freq' Hz (0 = silence). */
static void set_tone(uint16_t freq)
{
    if (freq == 0 || volume == 0) {
        pwm_set_gpio_level(AUDIO_PIN, 0);
        return;
    }
    uint32_t clk = clock_get_hz(clk_sys);

    /* Fractional divider so the 16-bit counter can reach low frequencies */
    float div = (float)clk / ((float)freq * 65536.0f);
    if (div < 1.0f) div = 1.0f;

    uint32_t wrap = (uint32_t)((float)clk / (div * (float)freq)) - 1;
    if (wrap > 65535) wrap = 65535;

    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, (uint16_t)wrap);
    /* 50% duty is loudest, so map volume 0-100 to duty 0-50% */
    pwm_set_gpio_level(AUDIO_PIN, (uint16_t)((wrap * volume) / 200));
}

static void start_note(void)
{
    note_t n = cur_notes[cur_idx];
    ms_left = n.ms;
    set_tone(n.freq);
}

/* 1 ms tick, runs in interrupt context */
static bool tick_cb(repeating_timer_t *t)
{
    (void)t;
    if (!playing) return true;

    if (ms_left > 1) {
        ms_left--;
        return true;
    }

    cur_idx++;
    if (cur_notes[cur_idx].ms == 0) {             /* reached NOTE_END */
        playing = false;
        set_tone(0);
    } else {
        start_note();
    }
    return true;                                   /* keep repeating */
}

void sound_init(void)
{
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(AUDIO_PIN);
    pwm_set_enabled(slice_num, true);
    pwm_set_gpio_level(AUDIO_PIN, 0);

    /* Negative delay = fixed 1 ms period measured between callback starts */
    add_repeating_timer_ms(-1, tick_cb, NULL, &tick_timer);
}

void sound_play(sfx_id_t id)
{
    if ((unsigned)id >= SFX_COUNT) return;

    /* Keep the tick ISR from seeing half-updated state. Nesting is fine,
     * so this is also safe when called from another interrupt handler. */
    uint32_t irq = save_and_disable_interrupts();

    if (playing && sfx_table[id].priority < cur_priority) {
        restore_interrupts(irq);                   /* current sound matters more */
        return;
    }
    cur_notes = sfx_table[id].notes;
    cur_priority = sfx_table[id].priority;
    cur_idx = 0;
    playing = true;
    start_note();

    restore_interrupts(irq);
}

void sound_stop(void)
{
    uint32_t irq = save_and_disable_interrupts();
    playing = false;
    set_tone(0);
    restore_interrupts(irq);
}

void sound_set_volume(uint8_t percent)
{
    volume = percent > 100 ? 100 : percent;
}

bool sound_is_playing(void)
{
    return playing;
}