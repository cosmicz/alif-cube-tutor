/* SPDX-License-Identifier: Apache-2.0
 * White-cross next-move planner for the AppKit LCD (arc-fdo1, 2026-10-18).
 *
 * C port of the host tutor's cross planner (curiosity_bench.domain.cube.cross_planner): a state is
 * 54 colour indices in face order U R F D L B (white, orange, blue, yellow, red, green), each face
 * row-major as seen from outside; a face turn is one of 18 permutations generated from the host
 * facelet model (face*3 + 0 clockwise, 1 counterclockwise, 2 half).  plan: iterative deepening up to
 * CUBE_PLAN_MAX_DEPTH turns, no same-face repeats, for the shortest sequence after which one more
 * white edge is home on U while every edge already home stays home.  Integer only, no allocation. */
#ifndef CUBE_CROSS_PLANNER_H
#define CUBE_CROSS_PLANNER_H
#include <stdint.h>
#include <string.h>
#include "cube_face_memory.h"

#define CUBE_PLAN_MAX_DEPTH 5
#define CUBE_FACELETS 54
#define CUBE_TURNS 18

static const uint8_t CUBE_TURN_PERM[CUBE_TURNS][CUBE_FACELETS] = {
    {6, 3, 0, 7, 4, 1, 8, 5, 2, 45, 46, 47, 12, 13, 14, 15, 16, 17, 9, 10, 11, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 18, 19, 20, 39, 40, 41, 42, 43, 44, 36, 37, 38, 48, 49, 50, 51, 52, 53},
    {2, 5, 8, 1, 4, 7, 0, 3, 6, 18, 19, 20, 12, 13, 14, 15, 16, 17, 36, 37, 38, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 45, 46, 47, 39, 40, 41, 42, 43, 44, 9, 10, 11, 48, 49, 50, 51, 52, 53},
    {8, 7, 6, 5, 4, 3, 2, 1, 0, 36, 37, 38, 12, 13, 14, 15, 16, 17, 45, 46, 47, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 9, 10, 11, 39, 40, 41, 42, 43, 44, 18, 19, 20, 48, 49, 50, 51, 52, 53},
    {0, 1, 20, 3, 4, 23, 6, 7, 26, 15, 12, 9, 16, 13, 10, 17, 14, 11, 18, 19, 29, 21, 22, 32, 24, 25, 35, 27, 28, 51, 30, 31, 48, 33, 34, 45, 36, 37, 38, 39, 40, 41, 42, 43, 44, 8, 46, 47, 5, 49, 50, 2, 52, 53},
    {0, 1, 51, 3, 4, 48, 6, 7, 45, 11, 14, 17, 10, 13, 16, 9, 12, 15, 18, 19, 2, 21, 22, 5, 24, 25, 8, 27, 28, 20, 30, 31, 23, 33, 34, 26, 36, 37, 38, 39, 40, 41, 42, 43, 44, 35, 46, 47, 32, 49, 50, 29, 52, 53},
    {0, 1, 29, 3, 4, 32, 6, 7, 35, 17, 16, 15, 14, 13, 12, 11, 10, 9, 18, 19, 51, 21, 22, 48, 24, 25, 45, 27, 28, 2, 30, 31, 5, 33, 34, 8, 36, 37, 38, 39, 40, 41, 42, 43, 44, 26, 46, 47, 23, 49, 50, 20, 52, 53},
    {0, 1, 2, 3, 4, 5, 44, 41, 38, 6, 10, 11, 7, 13, 14, 8, 16, 17, 24, 21, 18, 25, 22, 19, 26, 23, 20, 15, 12, 9, 30, 31, 32, 33, 34, 35, 36, 37, 27, 39, 40, 28, 42, 43, 29, 45, 46, 47, 48, 49, 50, 51, 52, 53},
    {0, 1, 2, 3, 4, 5, 9, 12, 15, 29, 10, 11, 28, 13, 14, 27, 16, 17, 20, 23, 26, 19, 22, 25, 18, 21, 24, 38, 41, 44, 30, 31, 32, 33, 34, 35, 36, 37, 8, 39, 40, 7, 42, 43, 6, 45, 46, 47, 48, 49, 50, 51, 52, 53},
    {0, 1, 2, 3, 4, 5, 29, 28, 27, 44, 10, 11, 41, 13, 14, 38, 16, 17, 26, 25, 24, 23, 22, 21, 20, 19, 18, 8, 7, 6, 30, 31, 32, 33, 34, 35, 36, 37, 15, 39, 40, 12, 42, 43, 9, 45, 46, 47, 48, 49, 50, 51, 52, 53},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 24, 25, 26, 18, 19, 20, 21, 22, 23, 42, 43, 44, 33, 30, 27, 34, 31, 28, 35, 32, 29, 36, 37, 38, 39, 40, 41, 51, 52, 53, 45, 46, 47, 48, 49, 50, 15, 16, 17},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 51, 52, 53, 18, 19, 20, 21, 22, 23, 15, 16, 17, 29, 32, 35, 28, 31, 34, 27, 30, 33, 36, 37, 38, 39, 40, 41, 24, 25, 26, 45, 46, 47, 48, 49, 50, 42, 43, 44},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 42, 43, 44, 18, 19, 20, 21, 22, 23, 51, 52, 53, 35, 34, 33, 32, 31, 30, 29, 28, 27, 36, 37, 38, 39, 40, 41, 15, 16, 17, 45, 46, 47, 48, 49, 50, 24, 25, 26},
    {53, 1, 2, 50, 4, 5, 47, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 0, 19, 20, 3, 22, 23, 6, 25, 26, 18, 28, 29, 21, 31, 32, 24, 34, 35, 42, 39, 36, 43, 40, 37, 44, 41, 38, 45, 46, 33, 48, 49, 30, 51, 52, 27},
    {18, 1, 2, 21, 4, 5, 24, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 27, 19, 20, 30, 22, 23, 33, 25, 26, 53, 28, 29, 50, 31, 32, 47, 34, 35, 38, 41, 44, 37, 40, 43, 36, 39, 42, 45, 46, 6, 48, 49, 3, 51, 52, 0},
    {27, 1, 2, 30, 4, 5, 33, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 53, 19, 20, 50, 22, 23, 47, 25, 26, 0, 28, 29, 3, 31, 32, 6, 34, 35, 44, 43, 42, 41, 40, 39, 38, 37, 36, 45, 46, 24, 48, 49, 21, 51, 52, 18},
    {11, 14, 17, 3, 4, 5, 6, 7, 8, 9, 10, 35, 12, 13, 34, 15, 16, 33, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 36, 39, 42, 2, 37, 38, 1, 40, 41, 0, 43, 44, 51, 48, 45, 52, 49, 46, 53, 50, 47},
    {42, 39, 36, 3, 4, 5, 6, 7, 8, 9, 10, 0, 12, 13, 1, 15, 16, 2, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 17, 14, 11, 33, 37, 38, 34, 40, 41, 35, 43, 44, 47, 50, 53, 46, 49, 52, 45, 48, 51},
    {35, 34, 33, 3, 4, 5, 6, 7, 8, 9, 10, 42, 12, 13, 39, 15, 16, 36, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 2, 1, 0, 17, 37, 38, 14, 40, 41, 11, 43, 44, 53, 52, 51, 50, 49, 48, 47, 46, 45},
};

/* Colour indices as the host model: W0 Y1 B2 G3 O4 R5. */
static inline int cube_colour_index(char letter)
{
    switch (letter) { case 'W': return 0; case 'Y': return 1; case 'B': return 2; case 'G': return 3; case 'O': return 4; case 'R': return 5; default: return -1; }
}

static const char *const CUBE_FACE_WORDS[6] = {"WHITE", "ORANGE", "BLUE", "YELLOW", "RED", "GREEN"};
static const char *const CUBE_TURN_WORDS[3] = {"CLOCKWISE", "COUNTERCLOCKWISE", "TWICE"};

static inline void cube_apply_turn(const uint8_t *state, int turn, uint8_t *out)
{
    const uint8_t *perm = CUBE_TURN_PERM[turn];
    for (int i = 0; i < CUBE_FACELETS; ++i) out[i] = state[perm[i]];
}

/* Build the model state from the face memory.  The kit's reads are taken in the mirrored LCD view,
 * so columns are reversed here to the model's seen-from-outside convention.  Returns 0 unless all
 * six faces are remembered. */
static inline int cube_state_from_memory(const cube_face_memory *m, uint8_t state[CUBE_FACELETS])
{
    if (m->seen != 0x3Fu) return 0;
    for (int face = 0; face < 6; ++face) {
        const char *letters = m->faces[face];   /* memory slots already follow U R F D L B */
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) {
                const int colour = cube_colour_index(letters[r * 3 + (2 - c)]);
                if (colour < 0) return 0;
                state[face * 9 + r * 3 + c] = (uint8_t)colour;
            }
    }
    return 1;
}

/* Bit i set when cross slot i (UF, UR, UB, UL) is home: U sticker = U centre, side sticker = side centre. */
static inline unsigned cube_cross_solved_mask(const uint8_t *s)
{
    static const uint8_t slots[4][2] = {{7, 2 * 9 + 1}, {5, 1 * 9 + 1}, {1, 5 * 9 + 1}, {3, 4 * 9 + 1}};
    unsigned mask = 0;
    for (int i = 0; i < 4; ++i) {
        const int side_face = slots[i][1] / 9;
        if (s[slots[i][0]] == s[4] && s[slots[i][1]] == s[side_face * 9 + 4]) mask |= 1u << i;
    }
    return mask;
}

static inline int cube_plan_search(const uint8_t *state, unsigned base, int depth, int last_face, uint8_t *plan)
{
    uint8_t next[CUBE_FACELETS];
    for (int turn = 0; turn < CUBE_TURNS; ++turn) {
        const int face = turn / 3;
        if (face == last_face) continue;
        cube_apply_turn(state, turn, next);
        if (depth == 1) {
            const unsigned mask = cube_cross_solved_mask(next);
            if ((mask & base) == base && mask != base) { plan[0] = (uint8_t)turn; return 1; }
            continue;
        }
        const int rest = cube_plan_search(next, base, depth - 1, face, plan + 1);
        if (rest) { plan[0] = (uint8_t)turn; return rest + 1; }
    }
    return 0;
}

/* Shortest sequence homing one more cross edge; returns its length, 0 when the cross is complete,
 * -1 when no plan exists within CUBE_PLAN_MAX_DEPTH. plan must hold CUBE_PLAN_MAX_DEPTH entries. */
static inline int cube_plan_cross(const uint8_t *state, uint8_t *plan)
{
    const unsigned base = cube_cross_solved_mask(state);
    if (base == 0xFu) return 0;
    for (int depth = 1; depth <= CUBE_PLAN_MAX_DEPTH; ++depth) {
        const int n = cube_plan_search(state, base, depth, -1, plan);
        if (n) return n;
    }
    return -1;
}

/* One line of guidance from the memory: the next turn in words, or why there is none. */
static inline int cube_plan_line(const cube_face_memory *m, char *out, size_t out_len)
{
    uint8_t state[CUBE_FACELETS], plan[CUBE_PLAN_MAX_DEPTH];
    if (!cube_state_from_memory(m, state)) { snprintf(out, out_len, "NEXT MOVE: SHOW ALL SIX FACES FIRST"); return -2; }
    const int n = cube_plan_cross(state, plan);
    if (n == 0) { snprintf(out, out_len, "WHITE CROSS DONE - NO MOVE NEEDED"); return 0; }
    if (n < 0) { snprintf(out, out_len, "NEXT MOVE: NO SHORT PLAN, CHECK THE FACES"); return -1; }
    snprintf(out, out_len, "NEXT MOVE: TURN THE %s FACE %s (%d TO HOME AN EDGE)",
             CUBE_FACE_WORDS[plan[0] / 3], CUBE_TURN_WORDS[plan[0] % 3], n);
    return n;
}
#endif
