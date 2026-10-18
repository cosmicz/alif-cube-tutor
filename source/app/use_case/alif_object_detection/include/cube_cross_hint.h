/* SPDX-License-Identifier: Apache-2.0
 * Single-face white-cross observation hint for the AppKit LCD (arc-fdo1, 2026-10-18, honest wording v2).
 *
 * Input: the nine letters the kit just read (row-major in the LCD view, 'W','Y','B','G','O','R',
 * anything else = unsure).  The centre letter says which face is shown, so one read is enough to
 * say where the white cross stands and what the standard beginner cue is for what is visible.
 * The hint only describes what is seen; it never claims a turn succeeded.  Pure C, no allocation. */
#ifndef CUBE_CROSS_HINT_H
#define CUBE_CROSS_HINT_H
#include <stddef.h>
#include <stdio.h>

#define CUBE_CROSS_HINT_LEN 112

static inline int cube_cross_is_colour(char c)
{
    return c == 'W' || c == 'Y' || c == 'B' || c == 'G' || c == 'O' || c == 'R';
}

static inline const char *cube_cross_colour_word(char c)
{
    switch (c) {
        case 'W': return "white"; case 'Y': return "yellow"; case 'B': return "blue";
        case 'G': return "green"; case 'O': return "orange"; case 'R': return "red";
        default: return "?";
    }
}

/* Writes one short line into out (at least CUBE_CROSS_HINT_LEN bytes); returns the number of white
 * edge stickers counted on the shown face (0..4), or -1 when no face colour is at the centre.
 * v3: lead with what the face shows; never ask to rotate.  The only pointer to another face is
 * implicit: when a white edge sticker is visible on a side or bottom edge, the arrow overlay shows
 * where its partner sticker is.  Rows/sides are as the camera sees the face; no orientation. */
static inline int cube_cross_hint(const char letters[9], char *out, size_t out_len)
{
    const char centre = letters[4];
    const int edge[4] = {1, 3, 5, 7};
    int whites = 0;
    if (!cube_cross_is_colour(centre)) {
        snprintf(out, out_len, "NO FACE CENTRE READ");
        return -1;
    }
    for (int i = 0; i < 4; ++i) whites += letters[edge[i]] == 'W';
    if (centre == 'W') {
        if (whites == 4) snprintf(out, out_len, "WHITE FACE: ALL 4 EDGE STICKERS WHITE");
        else snprintf(out, out_len, "WHITE FACE: %d OF 4 EDGE STICKERS WHITE", whites);
        return whites;
    }
    if (centre == 'Y') {
        if (whites) snprintf(out, out_len, "YELLOW FACE: %d WHITE EDGE STICKER%s HERE, THEY BELONG BY WHITE", whites, whites == 1 ? "" : "S");
        else snprintf(out, out_len, "YELLOW FACE: NO WHITE EDGE STICKERS HERE");
        return whites;
    }
    const char *word = cube_cross_colour_word(centre);
    if (letters[1] == 'W') { snprintf(out, out_len, "%s FACE: WHITE STICKER IN THE TOP EDGE", word); return whites; }
    if (letters[3] == 'W' || letters[5] == 'W') { snprintf(out, out_len, "%s FACE: WHITE STICKER IN A SIDE EDGE", word); return whites; }
    if (letters[7] == 'W') { snprintf(out, out_len, "%s FACE: WHITE STICKER IN THE BOTTOM EDGE", word); return whites; }
    if (letters[1] == centre) { snprintf(out, out_len, "%s FACE: TOP EDGE MATCHES THE CENTRE", word); return whites; }
    if (cube_cross_is_colour(letters[1])) snprintf(out, out_len, "%s FACE: TOP EDGE SHOWS %s", word, cube_cross_colour_word(letters[1]));
    else snprintf(out, out_len, "%s FACE: TOP EDGE UNCLEAR", word);
    return whites;
}

/* Where a visible white edge sticker's partner is: 0 none (no arrow), 1 the face above the shown
 * face, 2 the face below, 3 a face beside it.  Rows/sides are as the camera sees the face. */
static inline int cube_cross_hint_direction(const char letters[9])
{
    const char centre = letters[4];
    if (!cube_cross_is_colour(centre) || centre == 'W' || centre == 'Y') return 0;
    if (letters[1] == 'W') return 1;
    if (letters[3] == 'W' || letters[5] == 'W') return 3;
    if (letters[7] == 'W') return 2;
    return 0;  /* nothing to point at: no white edge sticker on this face */
}
#endif
