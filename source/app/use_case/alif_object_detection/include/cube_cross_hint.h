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
 * Honest by construction: one face shows one sticker of each edge, so the line only reports what
 * is visible and which face to show next; it never says an edge is placed, aligned or solved, and
 * it assumes no cube orientation (top/bottom mean rows of the face as the camera sees it). */
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
        if (whites == 4) snprintf(out, out_len, "WHITE FACE: 4/4 EDGE STICKERS WHITE - SHOW EACH SIDE TO CHECK THE OTHER STICKERS");
        else snprintf(out, out_len, "WHITE FACE: %d/4 EDGE STICKERS WHITE - THE OTHERS ARE ON OTHER FACES", whites);
        return whites;
    }
    if (centre == 'Y') {
        if (whites) snprintf(out, out_len, "YELLOW FACE: %d WHITE EDGE STICKER%s HERE - SHOW THE SIDE NEXT TO ONE", whites, whites == 1 ? "" : "S");
        else snprintf(out, out_len, "YELLOW FACE: NO WHITE EDGE STICKER HERE - SHOW A SIDE");
        return whites;
    }
    /* A side face: report its edge stickers; the matching sticker of each edge is on another face. */
    const char *word = cube_cross_colour_word(centre);
    if (letters[1] == 'W') { snprintf(out, out_len, "WHITE STICKER IN THE TOP EDGE OF %s - SHOW THE FACE ABOVE FOR ITS OTHER STICKER", word); return whites; }
    if (letters[3] == 'W' || letters[5] == 'W') { snprintf(out, out_len, "WHITE STICKER IN A SIDE EDGE OF %s - SHOW THE FACE BESIDE IT", word); return whites; }
    if (letters[7] == 'W') { snprintf(out, out_len, "WHITE STICKER IN THE BOTTOM EDGE OF %s - SHOW THE FACE BELOW FOR ITS OTHER STICKER", word); return whites; }
    if (letters[1] == centre) { snprintf(out, out_len, "TOP EDGE STICKER MATCHES THE %s CENTRE - SHOW THE FACE ABOVE TO SEE ITS OTHER STICKER", word); return whites; }
    if (cube_cross_is_colour(letters[1])) snprintf(out, out_len, "TOP EDGE OF %s SHOWS %s - SHOW THE FACE ABOVE TO SEE ITS OTHER STICKER", word, cube_cross_colour_word(letters[1]));
    else snprintf(out, out_len, "TOP EDGE OF %s UNCLEAR - HOLD STILL", word);
    return whites;
}
#endif
