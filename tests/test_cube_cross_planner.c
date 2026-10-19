/* SPDX-License-Identifier: Apache-2.0
 * Host regressions supplied with the Cube Tutor planner/memory handoff. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cube_face_memory.h"
#include "cube_cross_planner.h"

static void remember(cube_face_memory *m, const char *face)
{
    for (int i = 0; i < CUBE_MEMORY_STABLE_FRAMES; ++i) cube_face_memory_update(m, face);
}
static void solved_memory(cube_face_memory *m)
{
    cube_face_memory_reset(m);
    remember(m, "WWWWWWWWW"); remember(m, "OOOOOOOOO"); remember(m, "BBBBBBBBB");
    remember(m, "YYYYYYYYY"); remember(m, "RRRRRRRRR"); remember(m, "GGGGGGGGG");
}

int main(void)
{
    cube_face_memory m; char line[CUBE_MEMORY_LINE_LEN]; uint8_t state[54], next[54], plan[CUBE_PLAN_MAX_DEPTH];
    solved_memory(&m);
    assert(cube_state_from_memory(&m, state) && cube_cross_solved_mask(state) == 0xFu);
    assert(cube_plan_line(&m, line, sizeof line) == 0 && strstr(line, "DONE"));
    /* F from solved: the one-turn fix is F counterclockwise (turn id 2*3+1 = 7). */
    cube_apply_turn(state, 2 * 3 + 0, next);
    assert(cube_cross_solved_mask(next) == 0xEu);
    assert(cube_plan_cross(next, plan) == 1 && plan[0] == 7);
    /* Asymmetric face data must really be un-mirrored, not merely copied. */
    remember(&m, "ROBWWGYRO");
    assert(cube_state_from_memory(&m, state));
    assert(state[0] == cube_colour_index('B') && state[2] == cube_colour_index('R'));
    assert(state[3] == cube_colour_index('G') && state[5] == cube_colour_index('W'));
    assert(state[6] == cube_colour_index('O') && state[8] == cube_colour_index('Y'));
    /* Each turn is a permutation; four quarter turns restore an asymmetric state. */
    for (int face = 0; face < 6; ++face) {
        uint8_t unique[54], transformed[54];
        for (int i = 0; i < 54; ++i) unique[i] = (uint8_t)i;
        cube_apply_turn(unique, face * 3, transformed);
        assert(memcmp(unique, transformed, 54) != 0);
        for (int repeat = 1; repeat < 4; ++repeat) {
            memcpy(unique, transformed, 54);
            cube_apply_turn(unique, face * 3, transformed);
        }
        for (int i = 0; i < 54; ++i) assert(transformed[i] == i);
    }
    /* Scrambles: each plan homes one more edge, keeps the rest, and the cross completes. */
    unsigned seed = 12345;
    for (int trial = 0; trial < 20; ++trial) {
        memcpy(state, next, 54); int last = -1;
        for (int k = 0; k < 20; ++k) { seed = seed * 1103515245u + 12345u; int t = (int)((seed >> 8) % 18); if (t / 3 == last) t = (t + 3) % 18; cube_apply_turn(state, t, next); memcpy(state, next, 54); last = t / 3; }
        int plans = 0;
        while (cube_cross_solved_mask(state) != 0xFu) {
            const unsigned before = cube_cross_solved_mask(state);
            const int n = cube_plan_cross(state, plan);
            assert(n > 0 && n <= CUBE_PLAN_MAX_DEPTH);
            for (int i = 0; i < n; ++i) { cube_apply_turn(state, plan[i], next); memcpy(state, next, 54); }
            const unsigned after = cube_cross_solved_mask(state);
            assert((after & before) == before && after != before);
            assert(++plans <= 4);
        }
    }
    /* The memory line names a real turn; mirrored columns are undone on the way in. */
    solved_memory(&m);
    remember(&m, "WWWWWWBBB"); remember(&m, "WWWBBBBBB");  /* bottom row of white and top row of blue swapped: F2-like */
    assert(cube_plan_line(&m, line, sizeof line) > 0 && strstr(line, "NEXT MOVE: TURN THE"));
    cube_face_memory_reset(&m); remember(&m, "WWWWWWWWW");
    assert(cube_plan_line(&m, line, sizeof line) == -2 && strstr(line, "SIX FACES"));
    puts("PASS cross planner: solved, one-turn fix, 20 scrambles monotone, words, needs six faces");
    return 0;
}
