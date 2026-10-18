/* SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CUBE_PREVIEW_H
#define CUBE_PREVIEW_H

#include <stdint.h>

typedef struct { uint8_t r, g, b; char letter; } cube_colour;
static const cube_colour cube_palette[6] = {
    {235, 235, 235, 'W'}, {240, 220, 40, 'Y'}, {200, 30, 40, 'R'},
    {240, 120, 30, 'O'}, {30, 80, 200, 'B'}, {40, 170, 70, 'G'}
};

static inline unsigned cube_nearest_colour(unsigned r, unsigned g, unsigned b)
{
    unsigned best = 0, best_distance = UINT32_MAX;
    for (unsigned i = 0; i < 6; ++i) {
        int dr = (int)r - cube_palette[i].r;
        int dg = (int)g - cube_palette[i].g;
        int db = (int)b - cube_palette[i].b;
        unsigned distance = (unsigned)(dr * dr + dg * dg + db * db);
        if (distance < best_distance) { best = i; best_distance = distance; }
    }
    return best;
}

/* LVGL's full-panel RGB888 source is stored B,G,R, independently of the
 * RGB565 scanout. Sample the untouched source before drawing any overlays. */
static inline void cube_preview_read_and_draw(uint8_t *bgr, unsigned width,
                                              unsigned height, char letters[10])
{
    unsigned classes[9];
    const unsigned x0 = (width - 240) / 2, y0 = (height - 240) / 2;
    for (unsigned cell = 0; cell < 9; ++cell) {
        unsigned r = 0, g = 0, b = 0;
        unsigned cx = x0 + (cell % 3) * 80 + 40;
        unsigned cy = y0 + (cell / 3) * 80 + 40;
        for (unsigned y = cy - 5; y < cy + 5; ++y) {
            for (unsigned x = cx - 5; x < cx + 5; ++x) {
                const uint8_t *pixel = bgr + (y * width + x) * 3;
                b += pixel[0]; g += pixel[1]; r += pixel[2];
            }
        }
        classes[cell] = cube_nearest_colour(r / 100, g / 100, b / 100);
        letters[cell] = cube_palette[classes[cell]].letter;
    }
    letters[9] = '\0';
    for (unsigned y = 0; y < 240; ++y) {
        for (unsigned x = 0; x < 240; ++x) {
            if (x % 80 < 2 || y % 80 < 2 || x >= 238 || y >= 238) {
                uint8_t *pixel = bgr + ((y0 + y) * width + x0 + x) * 3;
                pixel[0] = pixel[1] = pixel[2] = 255;
            }
        }
    }
    for (unsigned cell = 0; cell < 9; ++cell) {
        const cube_colour *colour = &cube_palette[classes[cell]];
        unsigned left = x0 + (cell % 3) * 80 + 6;
        unsigned top = y0 + (cell / 3) * 80 + 6;
        for (unsigned y = top; y < top + 20; ++y) {
            for (unsigned x = left; x < left + 20; ++x) {
                uint8_t *pixel = bgr + (y * width + x) * 3;
                pixel[0] = colour->b; pixel[1] = colour->g; pixel[2] = colour->r;
            }
        }
    }
}
#endif
