#ifndef CUBE_BUTTON_ACTIONS_H
#define CUBE_BUTTON_ACTIONS_H
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool raw_pressed, stable_pressed, long_sent;
    uint32_t changed_ms, pressed_ms;
} cube_button_state;

/* Treat all joystick contacts as one action. Only a debounced all-released
 * interval re-arms a short press, so chords cannot CAL and LOCK at once. */
static inline uint32_t cube_button_action(cube_button_state *s, uint32_t mask,
                                         uint32_t now_ms, uint32_t read_mode)
{
    bool pressed = mask != 0;
    if (pressed != s->raw_pressed) {
        s->raw_pressed = pressed;
        s->changed_ms = now_ms;
    }
    if (pressed != s->stable_pressed && (uint32_t)(now_ms - s->changed_ms) >= 50) {
        s->stable_pressed = pressed;
        if (pressed) {
            s->pressed_ms = now_ms;
            s->long_sent = false;
        } else if (!s->long_sent) {
            return read_mode ? 2 : 1;
        }
    }
    if (s->stable_pressed && pressed && !s->long_sent &&
        (uint32_t)(now_ms - s->pressed_ms) >= 1500) {
        s->long_sent = true;
        return 2;
    }
    return 0;
}
#endif
