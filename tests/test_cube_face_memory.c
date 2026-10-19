/* SPDX-License-Identifier: Apache-2.0
 * Host regressions supplied with the Cube Tutor planner/memory handoff. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cube_face_memory.h"

static int stable(cube_face_memory *m, const char *face)
{
    int r = -1;
    for (int i = 0; i < CUBE_MEMORY_STABLE_FRAMES; ++i) r = cube_face_memory_update(m, face);
    return r;
}

int main(void)
{
    cube_face_memory m; cube_face_memory_reset(&m);
    char line[CUBE_MEMORY_LINE_LEN];
    assert(cube_face_memory_line(&m, line, sizeof line) == -1 && strstr(line, "FACES SEEN 0/6"));
    assert(cube_face_memory_update(&m, "W?WWWWWWW") == -1);
    assert(cube_face_memory_update(&m, "WWWWWWWWW") == -1);          /* one frame is not enough */
    assert(cube_face_memory_update(&m, "WWWWWWWWW") == -1);
    assert(cube_face_memory_update(&m, "WWWWWWWWW") == 0 && m.seen == 1u);
    assert(stable(&m, "BBBBBBBBB") == 2);
    assert(cube_face_memory_line(&m, line, sizeof line) == -1 && strstr(line, "FACES SEEN 2/6: W B"));
    assert(stable(&m, "OOOOOOOOO") == 1);
    assert(stable(&m, "RRRRRRRRR") == 4);
    assert(stable(&m, "GGGGGGGGG") == 5);
    assert(cube_face_memory_cross_known(&m));
    assert(cube_face_memory_line(&m, line, sizeof line) == 4 && strstr(line, "4/4"));
    /* A single flickered frame (orange centre read as red) must not touch the red face. */
    assert(cube_face_memory_update(&m, "OOOOROOOO") == -1);
    assert(cube_face_memory_line(&m, line, sizeof line) == 4);
    /* A flicker followed by two more identical frames IS stable and is stored (by its centre). */
    assert(cube_face_memory_update(&m, "OOOOROOOO") == -1);
    assert(cube_face_memory_update(&m, "OOOOROOOO") == 4);
    assert(cube_face_memory_line(&m, line, sizeof line) == 3 && strstr(line, "3/4") && strstr(line, "R(W/O)"));
    /* Restoring the red face returns to 4/4. */
    assert(stable(&m, "RRRRRRRRR") == 4);
    assert(cube_face_memory_line(&m, line, sizeof line) == 4);
    /* Flipped white-blue edge names both stickers. */
    assert(stable(&m, "WWWWWWWBW") == 0 && stable(&m, "BWBBBBBBB") == 2);
    assert(cube_face_memory_line(&m, line, sizeof line) == 3 && strstr(line, "B(B/W)"));
    assert(strlen(line) < CUBE_MEMORY_LINE_LEN - 1);
    puts("PASS face memory v2: three identical frames to remember, flicker ignored, stickers named");
    return 0;
}
