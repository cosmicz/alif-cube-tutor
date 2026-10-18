#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cube_cross_hint.h"

static int hint(const char *face, char *out) { return cube_cross_hint(face, out, CUBE_CROSS_HINT_LEN); }

int main(void)
{
    char out[CUBE_CROSS_HINT_LEN];
    assert(hint("WWWWWWWWW", out) == 4 && strstr(out, "4/4"));
    assert(hint("RWBWWGOYW", out) == 2 && strstr(out, "2/4"));           /* two white edges on top */
    assert(hint("?????????", out) == -1 && strstr(out, "SHOW ONE FACE"));
    assert(hint("YWYYYYYWY", out) == 2 && strstr(out, "BOTTOM") && strstr(out, "TWICE"));
    assert(hint("YYYYYYYYY", out) == 0 && strstr(out, "NO WHITE EDGE"));
    assert(hint("BBBBBBBBB", out) == 0 && strstr(out, "blue EDGE ALIGNED"));
    assert(hint("BWBBBBBBB", out) == 1 && strstr(out, "FLIPPED"));
    assert(hint("BGBBBBBBB", out) == 0 && strstr(out, "TOP EDGE ABOVE blue IS green"));
    assert(hint("BGBWBBBBB", out) == 1 && strstr(out, "MIDDLE LAYER"));
    assert(hint("BGBBBBBWB", out) == 1 && strstr(out, "AT BOTTOM OF blue"));
    assert(hint("B?BBBBBBB", out) == 0 && strstr(out, "UNCLEAR"));
    /* Every line fits the label buffer with its terminator. */
    const char *faces[] = {"WWWWWWWWW", "RWBWWGOYW", "YWYYYYYWY", "BWBBBBBBB", "BGBWBBBBB", "BGBBBBBWB"};
    for (unsigned i = 0; i < sizeof faces / sizeof *faces; ++i) {
        hint(faces[i], out);
        assert(strlen(out) < CUBE_CROSS_HINT_LEN);
    }
    puts("PASS single-face white-cross hint: top, bottom, side aligned/flipped/middle/bottom, unsure");
    return 0;
}
