/* SPDX-License-Identifier: Apache-2.0
 * Single-face white-cross tutoring hint for the AppKit LCD (arc-fdo1, 2026-10-18).
 *
 * Input: the nine letters the kit just read (row-major in the LCD view, 'W','Y','B','G','O','R',
 * anything else = unsure).  The centre letter says which face is shown, so one read is enough to
 * say where the white cross stands and what the standard beginner cue is for what is visible.
 * The hint only describes what is seen; it never claims a turn succeeded.  Pure C, no allocation. */
#ifndef CUBE_CROSS_HINT_H
#define CUBE_CROSS_HINT_H
#include <stddef.h>
#include <stdio.h>

#define CUBE_CROSS_HINT_LEN 96

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
 * edge stickers counted on the shown face (0..4), or -1 when no face colour is at the centre. */
static inline int cube_cross_hint(const char letters[9], char *out, size_t out_len)
{
    const char centre = letters[4];
    const int edge[4] = {1, 3, 5, 7};
    int whites = 0;
    if (!cube_cross_is_colour(centre)) {
        snprintf(out, out_len, "SHOW ONE FACE TO THE CAMERA");
        return -1;
    }
    for (int i = 0; i < 4; ++i) whites += letters[edge[i]] == 'W';
    if (centre == 'W') {
        if (whites == 4) snprintf(out, out_len, "WHITE CROSS 4/4 ON TOP - SHOW A SIDE TO CHECK IT");
        else snprintf(out, out_len, "WHITE CROSS %d/4 ON TOP - FIND WHITE EDGES ON SIDES AND BOTTOM", whites);
        return whites;
    }
    if (centre == 'Y') {
        if (whites) snprintf(out, out_len, "%d WHITE EDGE%s ON BOTTOM - TURN BOTTOM TO MATCH A SIDE, THEN THAT SIDE TWICE",
                             whites, whites == 1 ? "" : "S");
        else snprintf(out, out_len, "NO WHITE EDGE ON THE BOTTOM - CHECK THE SIDES");
        return whites;
    }
    /* A side face, white on top: the top-middle sticker belongs to the cross edge above this centre. */
    if (letters[1] == 'W') { snprintf(out, out_len, "FLIPPED WHITE EDGE ABOVE %s - THIS FACE, TOP AWAY, THEN RIGHT SIDE", cube_cross_colour_word(centre)); return whites; }
    if (letters[1] == centre) { snprintf(out, out_len, "%s EDGE ALIGNED ON TOP - SHOW THE NEXT SIDE", cube_cross_colour_word(centre)); return whites; }
    if (letters[3] == 'W' || letters[5] == 'W') { snprintf(out, out_len, "WHITE EDGE IN MIDDLE LAYER OF %s - TURN THIS FACE TO BRING IT UP", cube_cross_colour_word(centre)); return whites; }
    if (letters[7] == 'W') { snprintf(out, out_len, "WHITE EDGE AT BOTTOM OF %s - TURN BOTTOM AWAY, THEN THIS FACE TWICE", cube_cross_colour_word(centre)); return whites; }
    if (cube_cross_is_colour(letters[1])) snprintf(out, out_len, "TOP EDGE ABOVE %s IS %s - TURN THE TOP UNTIL COLOURS MATCH", cube_cross_colour_word(centre), cube_cross_colour_word(letters[1]));
    else snprintf(out, out_len, "TOP EDGE ABOVE %s UNCLEAR - HOLD STILL", cube_cross_colour_word(centre));
    return whites;
}
#endif
