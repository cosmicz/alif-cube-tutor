#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cube_cross_hint.h"

static int hint(const char *face, char *out) { return cube_cross_hint(face, out, CUBE_CROSS_HINT_LEN); }

int main(void)
{
    char out[CUBE_CROSS_HINT_LEN];
    assert(hint("WWWWWWWWW", out) == 4 && strstr(out, "ALL 4 EDGE STICKERS WHITE"));
    assert(hint("RWBWWGOYW", out) == 2 && strstr(out, "2 OF 4 EDGE STICKERS WHITE"));
    assert(hint("?????????", out) == -1 && strstr(out, "NO FACE CENTRE READ"));
    assert(hint("YWYYYYYWY", out) == 2 && strstr(out, "YELLOW FACE: 2 WHITE EDGE STICKERS"));
    assert(hint("YYYYYYYYY", out) == 0 && strstr(out, "NO WHITE EDGE"));
    assert(hint("BBBBBBBBB", out) == 0 && strstr(out, "blue FACE: TOP EDGE MATCHES THE CENTRE"));
    assert(hint("BWBBBBBBB", out) == 1 && strstr(out, "WHITE STICKER IN THE TOP EDGE"));
    assert(hint("BGBBBBBBB", out) == 0 && strstr(out, "blue FACE: TOP EDGE SHOWS green"));
    assert(hint("BGBWBBBBB", out) == 1 && strstr(out, "WHITE STICKER IN A SIDE EDGE"));
    assert(hint("BGBBBBBWB", out) == 1 && strstr(out, "WHITE STICKER IN THE BOTTOM EDGE"));
    assert(hint("B?BBBBBBB", out) == 0 && strstr(out, "UNCLEAR"));
    /* Every line fits the label buffer with its terminator. */
    const char *faces[] = {"WWWWWWWWW", "RWBWWGOYW", "YWYYYYYWY", "BWBBBBBBB", "BGBWBBBBB", "BGBBBBBWB"};
    for (unsigned i = 0; i < sizeof faces / sizeof *faces; ++i) {
        hint(faces[i], out);
        assert(strlen(out) < CUBE_CROSS_HINT_LEN);
        assert(!strstr(out, "ALIGNED") && !strstr(out, "SOLVED") && !strstr(out, "TURN "));
    }
    char tiny[2] = {'!', '!'};
    cube_cross_hint("WWWWWWWWW", tiny, sizeof tiny);
    assert(tiny[1] == '\0');
    puts("PASS single-face observations: counts, positions, uncertainty, no inferred turns");
    return 0;
}
