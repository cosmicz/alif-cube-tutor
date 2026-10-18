#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cube_preview.h"

static uint8_t model_rgb[24 * 24 * 3];
static uint8_t roi_bgr[240 * 240 * 3];

int main(void)
{
    static const uint8_t rgb[9][3] = {
        {235,235,235}, {240,220,40}, {200,30,40},
        {240,120,30}, {30,80,200}, {40,170,70},
        {200,30,40}, {240,220,40}, {235,235,235},
    };
    for (unsigned cell = 0; cell < 9; ++cell)
        for (unsigned y = 0; y < 6; ++y) for (unsigned x = 0; x < 6; ++x) {
            uint8_t *pixel = model_rgb + ((3 + cell / 3 * 6 + y) * 24 +
                                          3 + cell % 3 * 6 + x) * 3;
            memcpy(pixel, rgb[cell], 3);
        }
    assert(cube_roi_is_valid(24, 24, 3, 3, 18, 18));
    assert(!cube_roi_is_valid(24, 24, 3, 3, 14, 18));
    for (unsigned y = 0; y < 240; ++y) for (unsigned x = 0; x < 240; ++x) {
        const uint8_t *source = model_rgb + cube_roi_source_index(24, 3, 3, 18, 18, y, x) * 3;
        uint8_t *destination = roi_bgr + (y * 240 + x) * 3;
        destination[0] = source[2]; destination[1] = source[1]; destination[2] = source[0];
    }
    cube_diagnostics diagnostics = {.mode = 1, .white_valid = 1,
                                     .white_ref = {235, 235, 235}};
    char letters[10];
    cube_preview_read_and_draw(roi_bgr, 240, 240, letters, &diagnostics, 0, 0);
    assert(strcmp(letters, "WORYBYRGW") == 0);
    puts("PASS selected ROI transpose, calibrated colour readout, and invalid/no-detection guard");
}
