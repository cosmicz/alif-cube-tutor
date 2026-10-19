/* SPDX-License-Identifier: Apache-2.0
 * Face memory for the AppKit LCD cube tutor (arc-fdo1, 2026-10-18).
 *
 * Keeps the latest nine-letter read of each face, keyed by its centre colour, so the kit remembers
 * sides the operator has already shown.  Convention for assembly (shown on the LCD): hold white on
 * top; show each side with its top row touching white; show white by tilting up from the front and
 * yellow by tilting down from the front.  With that convention the faces map onto the facelet
 * model used by the host tutor: U white, R orange, F blue, D yellow, L red, B green; each 3x3 is
 * row-major as the camera sees it.  Pure C, no allocation. */
#ifndef CUBE_FACE_MEMORY_H
#define CUBE_FACE_MEMORY_H
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CUBE_FACE_COUNT 6
#define CUBE_MEMORY_LINE_LEN 112
#define CUBE_MEMORY_STABLE_FRAMES 3   /* identical reads in a row before a face is remembered */

/* Slot order follows the facelet model: U R F D L B = white orange blue yellow red green. */
static const char CUBE_FACE_LETTERS[CUBE_FACE_COUNT + 1] = "WOBYRG";

typedef struct {
    char faces[CUBE_FACE_COUNT][10];  /* nine letters + NUL per slot, '\0' first byte when unseen */
    unsigned seen;                    /* bit i set when slot i holds a read */
    char pending[10];                 /* the last complete read, awaiting confirmation */
    int pending_count;                /* how many times in a row pending was read */
} cube_face_memory;

static inline int cube_face_slot(char centre)
{
    for (int i = 0; i < CUBE_FACE_COUNT; ++i) if (CUBE_FACE_LETTERS[i] == centre) return i;
    return -1;
}

static inline void cube_face_memory_reset(cube_face_memory *m) { memset(m, 0, sizeof *m); }

/* Offer a read.  A read is complete when its centre and all nine cells are face colours; it is
 * remembered only after CUBE_MEMORY_STABLE_FRAMES identical complete reads in a row, so a single
 * flickered frame (a misread centre or cell) cannot overwrite a good face.  Returns the slot that
 * was (re)confirmed this frame, or -1 when nothing was stored. */
static inline int cube_face_memory_update(cube_face_memory *m, const char letters[9])
{
    const int slot = cube_face_slot(letters[4]);
    if (slot < 0) { m->pending_count = 0; return -1; }
    for (int i = 0; i < 9; ++i) if (cube_face_slot(letters[i]) < 0) { m->pending_count = 0; return -1; }
    if (m->pending_count > 0 && memcmp(m->pending, letters, 9) == 0) {
        ++m->pending_count;
    } else {
        memcpy(m->pending, letters, 9);
        m->pending[9] = '\0';
        m->pending_count = 1;
    }
    if (m->pending_count < CUBE_MEMORY_STABLE_FRAMES) return -1;
    memcpy(m->faces[slot], letters, 9);
    m->faces[slot][9] = '\0';
    m->seen |= 1u << slot;
    return slot;
}

static inline int cube_face_memory_count(const cube_face_memory *m)
{
    int n = 0;
    for (int i = 0; i < CUBE_FACE_COUNT; ++i) n += (m->seen >> i) & 1u;
    return n;
}

/* Cross slots on the white face with their partner sticker: (U cell, side slot, side cell). */
typedef struct { int u_cell; int side; int side_cell; } cube_cross_slot;
static const cube_cross_slot CUBE_CROSS_SLOTS[4] = {
    {7, 2, 1},  /* U bottom-middle  <-> F (blue)   top-middle */
    {5, 1, 1},  /* U right-middle   <-> R (orange) top-middle */
    {1, 5, 1},  /* U top-middle     <-> B (green)  top-middle */
    {3, 4, 1},  /* U left-middle    <-> L (red)    top-middle */
};

/* 1 when the white face and the four sides are remembered, else 0. */
static inline int cube_face_memory_cross_known(const cube_face_memory *m)
{
    return (m->seen & 0x37u) == 0x37u;  /* bits W O B R G = 0,1,2,4,5 */
}

/* Writes one status line: faces seen while incomplete, cross progress with partner stickers once
 * white and the four sides are known.  Returns solved cross slots (0..4) or -1 while unknown. */
static inline int cube_face_memory_line(const cube_face_memory *m, char *out, size_t out_len)
{
    if (!cube_face_memory_cross_known(m)) {
        char seen[16] = "";
        size_t n = 0;
        for (int i = 0; i < CUBE_FACE_COUNT; ++i) {
            if (!((m->seen >> i) & 1u)) continue;
            if (n) seen[n++] = ' ';
            seen[n++] = CUBE_FACE_LETTERS[i];
        }
        seen[n] = '\0';
        snprintf(out, out_len, "FACES SEEN %d/6%s%s - HOLD WHITE ON TOP", cube_face_memory_count(m),
                 n ? ": " : "", seen);
        return -1;
    }
    int solved = 0;
    char missing[64] = "";
    size_t n = 0;
    for (int s = 0; s < 4; ++s) {
        const cube_cross_slot c = CUBE_CROSS_SLOTS[s];
        const char side_centre = CUBE_FACE_LETTERS[c.side];
        const char up = m->faces[0][c.u_cell], side = m->faces[c.side][c.side_cell];
        if (up == 'W' && side == side_centre) { ++solved; continue; }
        /* Name the two stickers checked, so a misread face is visible: e.g. "O(W/R)" = white face
         * shows W, orange face top edge shows R. */
        n += (size_t)snprintf(missing + n, sizeof missing - n, "%s%c(%c/%c)", n ? " " : "", side_centre, up, side);
    }
    if (solved == 4) snprintf(out, out_len, "WHITE CROSS 4/4: ALL FOUR EDGES HOME");
    else snprintf(out, out_len, "WHITE CROSS %d/4: NOT HOME %s", solved, missing);
    return solved;
}
#endif
