#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "cube_preview.h"

/* Private table-calibration-20261018.LK7u1g, frame 2907.  The constants are
 * raw patch medians captured before white balancing; they are deliberately
 * shuffled below so expected labels cannot depend on a grid position. */
static const float captured_raw[9][3] = {
    {46.5277786f, 60.9861069f, 34.3333359f}, /* Y */
    {37.5000000f, 33.6944427f, 26.7500000f}, /* R */
    {36.3611107f, 34.0972214f, 27.2222214f}, /* R */
    {25.7222214f, 39.2777786f, 29.1250000f}, /* G */
    {34.4305573f, 31.1388893f, 25.6250000f}, /* R */
    {25.5138893f, 32.9583321f, 33.2777786f}, /* B */
    {34.2222214f, 31.4444447f, 25.5833321f}, /* O: RGB-overlaps red */
    {34.2777786f, 32.5000000f, 25.8888893f}, /* O: RGB-overlaps red */
    {43.0000000f, 54.6111107f, 32.6944466f}, /* Y */
};

static void fill_cell(uint8_t *image, unsigned cell, const float rgb[3])
{
    const unsigned x0 = cell % 3 * 80;
    const unsigned y0 = cell / 3 * 80;
    for (unsigned y = y0; y < y0 + 80; ++y)
        for (unsigned x = x0; x < x0 + 80; ++x) {
            uint8_t *p = image + (y * 240 + x) * 3;
            p[0] = (uint8_t)(rgb[2] + 0.5f); /* input is B, G, R */
            p[1] = (uint8_t)(rgb[1] + 0.5f);
            p[2] = (uint8_t)(rgb[0] + 0.5f);
        }
}

int main(void)
{
    static uint8_t image[240 * 240 * 3];
    static const unsigned source_for_cell[9] = {5, 0, 7, 3, 1, 8, 6, 4, 2};
    /* Orange samples remain R here: their RGB values overlap the confirmed
     * R sample, so a feature-only reader cannot honestly separate them. */
    static const char expected[] = "BYRGRYRRR";
    cube_diagnostics diagnostics = {
        .mode = 1,
        .white_valid = 1,
        .white_ref = {38.9652786f, 54.1666641f, 45.4027786f},
    };
    char letters[10];

    for (unsigned cell = 0; cell < 9; ++cell)
        fill_cell(image, cell, captured_raw[source_for_cell[cell]]);

    cube_preview_read_and_draw(image, 240, 240, letters, &diagnostics, 0, 0);
    assert(strcmp(letters, expected) == 0);
    assert(diagnostics.abstain_mask == 0);
    puts("PASS captured raw calibration: Y/R/G/B recover; red-orange unresolved");
}
