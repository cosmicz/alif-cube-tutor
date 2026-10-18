#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "cube_camera_mapping.h"

int main(void)
{
    /* Asymmetric sensor ABC/DEF -> clockwise DA/EB/FC -> mirror AD/BE/CF.
     * Every output row and all four corners are distinct. */
    const unsigned expected[6] = {0, 3, 1, 4, 2, 5};
    const unsigned old_mirror_only[6] = {2, 1, 0, 5, 4, 3};
    unsigned differs = 0;
    for (unsigned y = 0; y < 3; ++y)
        for (unsigned x = 0; x < 2; ++x) {
            unsigned index = cube_camera_source_index(3, 2, 2, 3, x, y);
            assert(index == expected[y * 2 + x]);
            differs += index != old_mirror_only[y * 2 + x];
        }
    assert(differs > 0); /* Deleting the rotation must fail the exact mapping. */
    /* Native 8x6 -> rotated6x8 -> central6x3 crop -> landscape8x4.
     * Literal indices independently encode clockwise source rows and crop. */
    const unsigned crop_expected[4][8] = {
        {2,2,10,18,26,26,34,42}, {2,2,10,18,26,26,34,42},
        {3,3,11,19,27,27,35,43}, {4,4,12,20,28,28,36,44}
    };
    for (unsigned y = 0; y < 4; ++y)
        for (unsigned x = 0; x < 8; ++x)
            assert(cube_camera_source_index(8, 6, 8, 4, x, y) == crop_expected[y][x]);
    /* An off-centre model rect maps through the same rotate/crop/mirror
     * transform: source x/y become logical y/x respectively. */
    cube_camera_point first = cube_camera_model_to_preview(480, 480, 800, 480,
                                                             192, 192, 48, 24);
    cube_camera_point last = cube_camera_model_to_preview(480, 480, 800, 480,
                                                            192, 192, 108, 54);
    assert(fabsf(first.x - 100) < 0.01f && fabsf(first.y - 40) < 0.01f);
    assert(fabsf(last.x - 225) < 0.01f && fabsf(last.y - 290) < 0.01f);
    puts("PASS clockwise camera + crop/mirror + off-centre model-to-preview box");
}
