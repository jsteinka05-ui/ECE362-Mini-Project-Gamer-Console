#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include <stdbool.h>

/* PWM audio pin. On the rev 1.2 PCB the speaker chain is:
 *   GPIO14 -> R3 (330k) / C3 (10nF) filter -> RV1 volume pot -> LM386 -> speaker
 * Override from platformio.ini with -DAUDIO_PIN=<n> to test on a bare Proton. */
#ifndef AUDIO_PIN
#define AUDIO_PIN 14
#endif

/* One ID per in-game event. To add a sound: add an ID here, a note list and
 * a table entry in sound.c. */
typedef enum {
    SFX_MENU_MOVE = 0,   /* menu cursor moves                  */
    SFX_MENU_SELECT,     /* menu item chosen / game start      */
    SFX_THROW,           /* ball launched (joystick button)    */
    SFX_CUP_HIT,         /* ball landed in a cup               */
    SFX_MISS,            /* ball missed every cup              */
    SFX_WIN,             /* all 10 cups cleared                */
    SFX_HIGH_SCORE,      /* new best score saved to EEPROM     */
    SFX_COUNT
} sfx_id_t;

/* Call once at startup (after stdio_init_all). */
void sound_init(void);

/* Start a sound. Non-blocking, and safe to call from main code OR from any
 * interrupt handler. A new sound replaces the current one unless the current
 * one has higher priority. */
void sound_play(sfx_id_t id);

/* Silence everything immediately. */
void sound_stop(void);

/* Software volume 0-100 (default 50). RV1 on the PCB is the hardware volume. */
void sound_set_volume(uint8_t percent);

/* True while a sound is still playing. */
bool sound_is_playing(void);

#endif