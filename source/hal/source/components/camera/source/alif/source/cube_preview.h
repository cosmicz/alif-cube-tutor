/* SPDX-FileCopyrightText: Copyright 2026 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CUBE_PREVIEW_H
#define CUBE_PREVIEW_H

#include <stdint.h>
#include <math.h>

typedef struct { uint8_t r, g, b; char letter; } cube_colour;
static const cube_colour cube_palette[7] = {
    {235, 235, 235, 'W'}, {240, 220, 40, 'Y'}, {200, 30, 40, 'R'},
    {240, 120, 30, 'O'}, {30, 80, 200, 'B'}, {40, 170, 70, 'G'},
    {128, 128, 128, '?'}
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

typedef struct {
    uint32_t magic;
    uint32_t frame_count;
    float gains[3];
    char raw_letters[10];
    char balanced_letters[10];
    float raw_rgb[9][3];
    float balanced_rgb[9][3];
    float hsv[9][3];
    float stddev[9];
    float hue_distance[9];
    uint32_t abstain_mask;
    uint32_t abstain_count;
} cube_diagnostics;

static inline float cube_clamp(float value, float low, float high)
{
    return value < low ? low : (value > high ? high : value);
}

static inline unsigned cube_hsv_colour(const float rgb[3], float hsv[3])
{
    float r = rgb[0] / 255.0f, g = rgb[1] / 255.0f, b = rgb[2] / 255.0f;
    float high = r > g ? r : g;
    if (b > high) high = b;
    float low = r < g ? r : g;
    if (b < low) low = b;
    float delta = high - low, hue = 0.0f;
    if (delta > 0.000001f) {
        if (high == r) hue = 60.0f * (g - b) / delta;
        else if (high == g) hue = 60.0f * (2.0f + (b - r) / delta);
        else hue = 60.0f * (4.0f + (r - g) / delta);
        if (hue < 0) hue += 360.0f;
    }
    hsv[0] = hue;
    hsv[1] = high > 0 ? delta / high : 0;
    hsv[2] = high;
    if (hsv[1] < 0.35f && high > 0.55f) return 0;
    if (hue >= 340 || hue < 20) return 2;
    if (hue < 45) return 3;
    if (hue < 75) return 1;
    if (hue < 170) return 5;
    if (hue < 270) return 4;
    /* Purple is outside the supplied bands; choose the nearer blue/red band. */
    return hue < 305 ? 4 : 2;
}

/* LVGL's full-panel RGB888 source is stored B,G,R, independently of the
 * RGB565 scanout. Sample the untouched source before drawing any overlays. */
static inline void cube_preview_read_and_draw(uint8_t *bgr, unsigned width,
                                              unsigned height, char letters[10],
                                              cube_diagnostics *diagnostics)
{
    unsigned classes[9];
    uint32_t totals[3] = {0, 0, 0};
    unsigned balance_pixels = 0;
    diagnostics->abstain_mask = diagnostics->abstain_count = 0;
    const unsigned x0 = (width - 240) / 2, y0 = (height - 240) / 2;
    for (unsigned cell = 0; cell < 9; ++cell) {
        unsigned r = 0, g = 0, b = 0;
        uint32_t squares[3] = {0, 0, 0};
        unsigned cx = x0 + (cell % 3) * 80 + 40;
        unsigned cy = y0 + (cell / 3) * 80 + 40;
        for (unsigned y = cy - 5; y < cy + 5; ++y) {
            for (unsigned x = cx - 5; x < cx + 5; ++x) {
                const uint8_t *pixel = bgr + (y * width + x) * 3;
                b += pixel[0]; g += pixel[1]; r += pixel[2];
                squares[0] += pixel[2] * pixel[2];
                squares[1] += pixel[1] * pixel[1];
                squares[2] += pixel[0] * pixel[0];
            }
        }
        diagnostics->raw_rgb[cell][0] = r / 100.0f;
        diagnostics->raw_rgb[cell][1] = g / 100.0f;
        diagnostics->raw_rgb[cell][2] = b / 100.0f;
        float max_variance = 0;
        for (unsigned c = 0; c < 3; ++c) {
            float mean = diagnostics->raw_rgb[cell][c];
            float variance = squares[c] / 100.0f - mean * mean;
            if (variance > max_variance) max_variance = variance;
        }
        /* Maximum per-channel patch standard deviation is conservative. */
        diagnostics->stddev[cell] = sqrtf(max_variance);
        totals[0] += r; totals[1] += g; totals[2] += b;
        balance_pixels += 100;
        unsigned raw_class = cube_nearest_colour(r / 100, g / 100, b / 100);
        diagnostics->raw_letters[cell] = cube_palette[raw_class].letter;
    }
    /* Estimate gains from nine sticker patches plus the unpainted perimeter,
     * avoiding the mostly-red guide interior and any synthetic overlay. */
    for (unsigned y = 0; y < 240; ++y) {
        for (unsigned x = 0; x < 240; ++x) {
            if (x != 0 && x != 239 && y != 0 && y != 239) continue;
            const uint8_t *p = bgr + ((y0 + y) * width + x0 + x) * 3;
            totals[0] += p[2]; totals[1] += p[1]; totals[2] += p[0];
            ++balance_pixels;
        }
    }
    float overall = (totals[0] + totals[1] + totals[2]) / 3.0f;
    for (unsigned c = 0; c < 3; ++c)
        diagnostics->gains[c] = totals[c] ? cube_clamp(overall / totals[c], 0.5f, 2.0f) : 2.0f;
    (void)balance_pixels; /* Equal pixel counts cancel in the gain ratios. */
    for (unsigned cell = 0; cell < 9; ++cell) {
        for (unsigned c = 0; c < 3; ++c)
            diagnostics->balanced_rgb[cell][c] = cube_clamp(
                diagnostics->raw_rgb[cell][c] * diagnostics->gains[c], 0, 255);
        classes[cell] = cube_hsv_colour(diagnostics->balanced_rgb[cell], diagnostics->hsv[cell]);
        const float *hsv = diagnostics->hsv[cell];
        float distance = 0;
        if (classes[cell] != 0) {
            const float palette_hues[5] = {356.5f, 25.7f, 54.0f, 133.85f, 222.35f};
            distance = 180;
            for (unsigned i = 0; i < 5; ++i) {
                float d = fabsf(hsv[0] - palette_hues[i]);
                if (d > 180) d = 360 - d;
                if (d < distance) distance = d;
            }
        }
        diagnostics->hue_distance[cell] = distance;
        if ((hsv[1] < 0.2f && hsv[2] < 0.55f) ||
            diagnostics->stddev[cell] > 28 || distance > 20) {
            classes[cell] = 6;
            diagnostics->abstain_mask |= 1u << cell;
            ++diagnostics->abstain_count;
        }
        letters[cell] = cube_palette[classes[cell]].letter;
        diagnostics->balanced_letters[cell] = letters[cell];
    }
    letters[9] = '\0';
    diagnostics->raw_letters[9] = diagnostics->balanced_letters[9] = '\0';
    diagnostics->magic = 0x43554245;
    ++diagnostics->frame_count;
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
                if (classes[cell] == 6 &&
                    ((x - left) == (y - top) || (x - left) + (y - top) == 19))
                    pixel[0] = pixel[1] = pixel[2] = 0;
            }
        }
    }
}
#endif
