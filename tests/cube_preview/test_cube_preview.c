#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#define RTE_PANEL_HACTIVE_TIME 480
#define RTE_PANEL_VACTIVE_LINE 800
#include "cube_display_geometry.h"
#include "cube_preview.h"

static uint8_t image[CUBE_PREVIEW_WIDTH * CUBE_PREVIEW_HEIGHT * 3];
static void fill_cell(unsigned cell, unsigned r, unsigned g, unsigned b)
{
    unsigned x0 = (CUBE_PREVIEW_WIDTH - 240) / 2 + cell % 3 * 80;
    unsigned y0 = (CUBE_PREVIEW_HEIGHT - 240) / 2 + cell / 3 * 80;
    for (unsigned y = y0; y < y0 + 80; ++y)
        for (unsigned x = x0; x < x0 + 80; ++x) {
            uint8_t *p = image + (y * CUBE_PREVIEW_WIDTH + x) * 3;
            p[0] = b; p[1] = g; p[2] = r;
        }
}
static void white_cross(void)
{
    memset(image, 128, sizeof image);
    for (unsigned cell = 0; cell < 9; ++cell) fill_cell(cell, 180, 200, 220);
    fill_cell(0, 200, 30, 40); fill_cell(2, 40, 170, 70);
    fill_cell(6, 30, 80, 200); fill_cell(8, 240, 120, 30);
    fill_cell(4, 30, 80, 200);
}
static void read_frame(cube_diagnostics *d, char letters[10], uint32_t request)
{
    cube_preview_read_and_draw(image, CUBE_PREVIEW_WIDTH, CUBE_PREVIEW_HEIGHT,
                              letters, d, 100, request);
}
int main(void)
{
    cube_diagnostics d = {.mode = 1};
    char letters[10];
    white_cross(); read_frame(&d, letters, 0);
    assert(d.mode == 1 && !d.white_valid); /* Boot must not auto-lock. */
    white_cross(); read_frame(&d, letters, 2);
    assert(d.mode == 0 && d.cross_valid && d.reference_mask == 0xAA);
    assert(d.candidate_rgb[0] == 180 && d.candidate_rgb[1] == 200);
    white_cross(); read_frame(&d, letters, 1);
    assert(d.white_valid && d.logo_ref_valid && d.mode == 1);
    assert(d.white_success_count == 1 && d.white_ref[0] == 180);
    assert(d.logo_ref_raw[0] == 30 && d.logo_ref_raw[1] == 80);
    assert(fabsf(d.logo_ref_rgb[0] - 30 * 235.0f / 180) < 0.001f);
    white_cross(); read_frame(&d, letters, 0);
    assert(letters[6] == 'W' && (d.logo_mask & (1u << 6)));
    white_cross(); fill_cell(6, 20, 40, 240); read_frame(&d, letters, 0);
    assert(letters[6] == 'B' && !(d.logo_mask & (1u << 6)));
    white_cross(); fill_cell(4, 180, 200, 220);
    unsigned cx = CUBE_PREVIEW_WIDTH / 2, cy = CUBE_PREVIEW_HEIGHT / 2;
    for (unsigned y = cy - 5; y < cy + 5; ++y)
        for (unsigned x = cx - 5; x < cx + 5; ++x) {
            uint8_t *p = image + (y * CUBE_PREVIEW_WIDTH + x) * 3;
            p[0] = 200; p[1] = 80; p[2] = 30;
        }
    read_frame(&d, letters, 0);
    assert(d.raw_rgb[4][0] == 180 && d.raw_rgb[4][1] == 200);
    assert(image[0] != 128); /* Gains visibly transform preview pixels. */
    white_cross(); fill_cell(1, 200, 30, 40); read_frame(&d, letters, 2);
    assert(d.mode == 0 && !d.cross_valid && !d.logo_ref_valid);
    white_cross(); fill_cell(1, 200, 30, 40); read_frame(&d, letters, 1);
    assert(d.mode == 0 && !d.white_valid && d.lock_rejected);
    white_cross(); read_frame(&d, letters, 1);
    assert(d.mode == 1 && d.white_valid && !d.lock_rejected);
    assert(sizeof(cube_diagnostics) == 736 && offsetof(cube_diagnostics, logo_ref_raw) == 592);
    assert(CUBE_STATUS_Y > (CUBE_PREVIEW_HEIGHT + 240) / 2);
    assert(CUBE_STATUS_Y + 80 <= CUBE_PREVIEW_HEIGHT);
    printf("PASS rotation=%d canvas=%dx%d white-cross/logo/reject/relock/visible-WB\n",
           CUBE_DISPLAY_ROTATION, CUBE_PREVIEW_WIDTH, CUBE_PREVIEW_HEIGHT);
}
