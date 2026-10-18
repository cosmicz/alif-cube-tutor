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
    float white_ref[3];
    uint32_t mode; /* 0: CALIBRATION, 1: READ */
    uint32_t white_valid;
    uint32_t white_streak;
    uint32_t abstain_since_ms;
    uint32_t white_success_count;
    float corner_rgb[4][3];
    float corner_hsv[4][3];
    float candidate_rgb[3];
    uint32_t logo_mask;
    uint32_t reference_mask;
    float logo_ref_raw[3];
    float logo_ref_rgb[3];
    float logo_ref_hsv[3];
    uint32_t logo_ref_valid;
    uint32_t cross_valid;
    uint32_t lock_rejected;
    float cross_balanced_rgb[4][3];
    float cross_hsv[4][3];
} cube_diagnostics;

static inline float cube_median4(const float input[4])
{
    float values[4] = {input[0], input[1], input[2], input[3]};
    for (unsigned i = 1; i < 4; ++i) {
        float value = values[i];
        unsigned j = i;
        while (j && values[j - 1] > value) { values[j] = values[j - 1]; --j; }
        values[j] = value;
    }
    return (values[1] + values[2]) / 2;
}

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
                                              cube_diagnostics *diagnostics,
                                              uint32_t now_ms, uint32_t calibration_request)
{
    unsigned classes[9];
    uint32_t white_success_before = diagnostics->white_success_count;
    uint32_t totals[3] = {0, 0, 0};
    unsigned balance_pixels = 0;
    diagnostics->abstain_mask = diagnostics->abstain_count = 0;
    diagnostics->logo_mask = 0;
    diagnostics->reference_mask = diagnostics->mode == 0 ? 0xAA : 0;
    const unsigned x0 = (width - 240) / 2, y0 = (height - 240) / 2;
    for (unsigned cell = 0; cell < 9; ++cell) {
        unsigned cx = x0 + (cell % 3) * 80 + 40;
        unsigned cy = y0 + (cell / 3) * 80 + 40;
        float patch_means[3][4], patch_variances[3][4];
        for (unsigned patch = 0; patch < 4; ++patch) {
            unsigned px = cx + ((patch & 1) ? 24 : -24);
            unsigned py = cy + ((patch & 2) ? 24 : -24);
            uint32_t sum[3] = {0, 0, 0}, squares[3] = {0, 0, 0};
            for (unsigned y = py - 3; y < py + 3; ++y) {
                for (unsigned x = px - 3; x < px + 3; ++x) {
                    const uint8_t *p = bgr + (y * width + x) * 3;
                    for (unsigned c = 0; c < 3; ++c) {
                        unsigned value = p[2 - c];
                        sum[c] += value; squares[c] += value * value;
                    }
                }
            }
            for (unsigned c = 0; c < 3; ++c) {
                patch_means[c][patch] = sum[c] / 36.0f;
                patch_variances[c][patch] = squares[c] / 36.0f -
                    patch_means[c][patch] * patch_means[c][patch];
            }
        }
        float max_variance = 0;
        for (unsigned c = 0; c < 3; ++c) {
            diagnostics->raw_rgb[cell][c] = cube_median4(patch_means[c]);
            float variance = cube_median4(patch_variances[c]);
            if (variance > max_variance) max_variance = variance;
            totals[c] += (uint32_t)(diagnostics->raw_rgb[cell][c] * 100);
        }
        /* Median patch variance rejects one logo/highlight outlier. */
        diagnostics->stddev[cell] = sqrtf(max_variance);
        balance_pixels += 100;
        unsigned raw_class = cube_nearest_colour(diagnostics->raw_rgb[cell][0],
            diagnostics->raw_rgb[cell][1], diagnostics->raw_rgb[cell][2]);
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
    if (calibration_request == 2) {
        diagnostics->mode = 0;
        diagnostics->white_valid = 0;
        diagnostics->logo_ref_valid = 0;
        diagnostics->white_streak = 0;
        diagnostics->lock_rejected = 0;
    }
    if (diagnostics->mode == 0) {
        const unsigned edges[4] = {1, 3, 5, 7};
        unsigned qualified = 0;
        for (unsigned corner = 0; corner < 4; ++corner) {
            for (unsigned c = 0; c < 3; ++c)
                diagnostics->corner_rgb[corner][c] = diagnostics->raw_rgb[edges[corner]][c];
            cube_hsv_colour(diagnostics->corner_rgb[corner], diagnostics->corner_hsv[corner]);
            if (diagnostics->corner_hsv[corner][1] < 0.35f &&
                diagnostics->corner_hsv[corner][2] > 0.4f) ++qualified;
        }
        for (unsigned c = 0; c < 3; ++c) {
            float edge_values[4];
            for (unsigned i = 0; i < 4; ++i) edge_values[i] = diagnostics->corner_rgb[i][c];
            diagnostics->candidate_rgb[c] = cube_median4(edge_values);
        }
        diagnostics->cross_valid = 1;
        for (unsigned edge = 0; edge < 4; ++edge) {
            for (unsigned c = 0; c < 3; ++c) {
                float candidate = diagnostics->candidate_rgb[c];
                float value = diagnostics->corner_rgb[edge][c];
                diagnostics->cross_balanced_rgb[edge][c] = cube_clamp(
                    value * 235.0f / (candidate > 1 ? candidate : 1), 0, 255);
                if (candidate < 1 || fabsf(value - candidate) > 0.15f * candidate)
                    diagnostics->cross_valid = 0;
            }
            cube_hsv_colour(diagnostics->cross_balanced_rgb[edge], diagnostics->cross_hsv[edge]);
            if (diagnostics->cross_hsv[edge][1] >= 0.25f ||
                diagnostics->cross_hsv[edge][2] <= 0.5f) diagnostics->cross_valid = 0;
        }
        diagnostics->white_streak = qualified == 4 ? diagnostics->white_streak + 1 : 0;
        if (calibration_request == 1 && diagnostics->cross_valid) {
            for (unsigned c = 0; c < 3; ++c)
                diagnostics->white_ref[c] = diagnostics->candidate_rgb[c];
            diagnostics->white_valid = 1;
            diagnostics->mode = 1;
            diagnostics->abstain_since_ms = 0;
            diagnostics->lock_rejected = 0;
            ++diagnostics->white_success_count;
        } else if (calibration_request == 1) diagnostics->lock_rejected = 1;
    }
    for (unsigned c = 0; c < 3; ++c)
        diagnostics->gains[c] = diagnostics->white_valid
            ? 235.0f / (diagnostics->white_ref[c] > 1 ? diagnostics->white_ref[c] : 1)
            : (totals[c] ? cube_clamp(overall / totals[c], 0.5f, 2.0f) : 2.0f);
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
    }
    if (white_success_before != diagnostics->white_success_count) {
        for (unsigned c = 0; c < 3; ++c) {
            diagnostics->logo_ref_raw[c] = diagnostics->raw_rgb[4][c];
            diagnostics->logo_ref_rgb[c] = diagnostics->balanced_rgb[4][c];
            diagnostics->logo_ref_hsv[c] = diagnostics->hsv[4][c];
        }
        diagnostics->logo_ref_valid = 1;
    }
    if (diagnostics->logo_ref_valid) {
        for (unsigned cell = 0; cell < 9; ++cell) {
            if (classes[cell] != 6 && classes[cell] != 4 && diagnostics->hsv[cell][2] >= 0.55f)
                continue;
            float distance_squared = 0;
            for (unsigned c = 0; c < 3; ++c) {
                float delta = diagnostics->balanced_rgb[cell][c] - diagnostics->logo_ref_rgb[c];
                distance_squared += delta * delta;
            }
            if (distance_squared < 25.0f * 25.0f) {
                if (classes[cell] == 6) {
                    diagnostics->abstain_mask &= ~(1u << cell);
                    --diagnostics->abstain_count;
                }
                classes[cell] = 0;
                diagnostics->logo_mask |= 1u << cell;
            }
        }
    }
    unsigned white_others = 0;
    for (unsigned cell = 0; cell < 9; ++cell)
        if (cell != 4 && classes[cell] == 0) ++white_others;
    if (diagnostics->mode == 0) diagnostics->logo_mask |= 1u << 4;
    if (white_others == 8 && classes[4] != 0) {
        if (classes[4] == 6) {
            diagnostics->abstain_mask &= ~(1u << 4);
            --diagnostics->abstain_count;
        }
        classes[4] = 0;
        diagnostics->logo_mask |= 1u << 4;
    }
    for (unsigned cell = 0; cell < 9; ++cell) {
        letters[cell] = cube_palette[classes[cell]].letter;
        diagnostics->balanced_letters[cell] = letters[cell];
    }
    letters[9] = '\0';
    diagnostics->raw_letters[9] = diagnostics->balanced_letters[9] = '\0';
    (void)now_ms; /* White-cross UX enters calibration only through BACK. */
    diagnostics->reference_mask = diagnostics->mode == 0 ? 0xAA : 0;
    diagnostics->magic = 0x43554245;
    ++diagnostics->frame_count;
    /* Apply the same gain to the visible camera preview, before drawing
     * guide/swatch UI. Diagnostics retain the original and corrected samples. */
    uint32_t gain_q8[3];
    for (unsigned c = 0; c < 3; ++c) gain_q8[c] = (uint32_t)(diagnostics->gains[c] * 256);
    for (unsigned i = 0; i < width * height; ++i) {
        uint8_t *p = bgr + i * 3;
        for (unsigned c = 0; c < 3; ++c) {
            uint32_t value = (p[2 - c] * gain_q8[c] + 128) >> 8;
            p[2 - c] = value > 255 ? 255 : value;
        }
    }
    for (unsigned y = 0; y < 240; ++y) {
        for (unsigned x = 0; x < 240; ++x) {
            if (x % 80 < 2 || y % 80 < 2 || x >= 238 || y >= 238) {
                uint8_t *pixel = bgr + ((y0 + y) * width + x0 + x) * 3;
                pixel[0] = pixel[1] = pixel[2] = 255;
            }
        }
    }
    if (diagnostics->reference_mask) {
        for (unsigned cell = 1; cell < 8; cell += 2) {
            unsigned left = x0 + cell % 3 * 80, top = y0 + cell / 3 * 80;
            for (unsigned y = 2; y < 78; ++y)
                for (unsigned x = 2; x < 78; ++x)
                    if (x < 4 || x >= 76 || y < 4 || y >= 76) {
                        uint8_t *p = bgr + ((top + y) * width + left + x) * 3;
                        p[0] = 0; p[1] = p[2] = 255;
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
                if ((diagnostics->logo_mask & (1u << cell)) &&
                    x - left >= 9 && x - left <= 11 && y - top >= 9 && y - top <= 11)
                    pixel[0] = pixel[1] = pixel[2] = 0;
                if (classes[cell] == 6 &&
                    ((x - left) == (y - top) || (x - left) + (y - top) == 19))
                    pixel[0] = pixel[1] = pixel[2] = 0;
            }
        }
    }
}
#endif
