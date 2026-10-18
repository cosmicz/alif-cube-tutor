#include <assert.h>
#include <stdio.h>
#include "cube_button_actions.h"

static void short_press(cube_button_state *s, unsigned mask, unsigned now, unsigned mode)
{
    assert(cube_button_action(s, mask, now, mode) == 0);
    assert(cube_button_action(s, mask, now + 50, mode) == 0);
    assert(cube_button_action(s, mask, now + 500, mode) == 0);
    assert(cube_button_action(s, 0, now + 550, mode) == 0);
    assert(cube_button_action(s, 0, now + 600, mode) == (mode ? 2u : 1u));
    assert(cube_button_action(s, 0, now + 800, mode) == 0);
}
int main(void)
{
    for (unsigned button = 0; button < 5; ++button) {
        cube_button_state s = {0};
        short_press(&s, 1u << button, 100, 1); /* READ -> CAL. */
        short_press(&s, 1u << button, 1100, 0); /* CAL -> validated lock request. */
    }
    cube_button_state s = {0};
    assert(cube_button_action(&s, 1, 100, 1) == 0);
    assert(cube_button_action(&s, 1, 150, 1) == 0);
    assert(cube_button_action(&s, 3, 500, 1) == 0); /* Chord does not re-arm. */
    assert(cube_button_action(&s, 2, 700, 1) == 0); /* First button released. */
    assert(cube_button_action(&s, 0, 800, 1) == 0);
    assert(cube_button_action(&s, 0, 850, 1) == 2);
    assert(cube_button_action(&s, 0, 900, 0) == 0); /* No double transition. */
    short_press(&s, 31, 1000, 0); /* All-five chord: one lock request. */
    s = (cube_button_state){0};
    assert(cube_button_action(&s, 4, 100, 1) == 0);
    assert(cube_button_action(&s, 0, 120, 1) == 0);
    assert(cube_button_action(&s, 0, 200, 1) == 0); /* Bounce ignored. */
    assert(cube_button_action(&s, 4, 300, 1) == 0);
    assert(cube_button_action(&s, 4, 350, 1) == 0);
    assert(cube_button_action(&s, 4, 1850, 1) == 2);
    assert(cube_button_action(&s, 12, 2200, 0) == 0);
    assert(cube_button_action(&s, 0, 2300, 0) == 0);
    assert(cube_button_action(&s, 0, 2350, 0) == 0); /* Long release cannot lock. */
    short_press(&s, 16, 2500, 0);
    s = (cube_button_state){0};
    assert(cube_button_action(&s, 1, 100, 1) == 0);
    assert(cube_button_action(&s, 1, 150, 1) == 0);
    assert(cube_button_action(&s, 0, 200, 1) == 0);
    assert(cube_button_action(&s, 2, 220, 1) == 0); /* Not fully released for50ms. */
    assert(cube_button_action(&s, 2, 300, 1) == 0);
    assert(cube_button_action(&s, 0, 400, 1) == 0);
    assert(cube_button_action(&s, 0, 450, 1) == 2);
    puts("PASS five buttons, held/chord, debounce/release, long-hold one-shot");
}
