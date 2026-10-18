#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cube_preview.h"

static void expect(const float rgb[3], unsigned colour)
{
    float hsv[3];
    assert(cube_hsv_colour(rgb, hsv) == colour);
}
static void pipeline(unsigned r, unsigned g, unsigned b, char expected)
{
    static uint8_t image[240 * 240 * 3];
    for (unsigned i = 0; i < 240 * 240; ++i) {
        image[i * 3] = b; image[i * 3 + 1] = g; image[i * 3 + 2] = r;
    }
    cube_diagnostics d = {.mode = 1, .white_valid = 1,
        .white_ref = {126.8611145f, 137.2430420f, 77.6527786f}};
    char letters[10];
    cube_preview_read_and_draw(image, 240, 240, letters, &d, 0, 0);
    for (unsigned i = 0; i < 9; ++i) assert(letters[i] == expected);
}
int main(void)
{
    /* Exact archived green-white frame840 corrected RGB; red previously W. */
    expect((float[3]){167.3609467f,108.9920654f,122.5648422f}, 2);
    expect((float[3]){85.7001266f,97.9335175f,138.4949036f}, 4);
    /* Captured dark green is not white; pipeline must continue to ABSTAIN,
     * not invent a green hue remapping for the blue-shifted measurement. */
    float hsv[3];
    assert(cube_hsv_colour((float[3]){94.2418442f,119.1706772f,126.8941269f}, hsv) != 0);
    expect((float[3]){255,255,255}, 0);
    expect((float[3]){253.8586578f,255,255}, 0);
    expect((float[3]){220.4832306f,217.9200745f,222.1361999f}, 0);
    expect((float[3]){130,180,140}, 5); /* Chromatic green cannot become W. */
    pipeline(90,64,40,'R'); /* Rounded captured red raw RGB + SAME saved gains. */
    pipeline(46,57,46,'B');
    pipeline(51,70,42,'?');
    pipeline(150,167,95,'W'); /* Clipped-white ambiguity remains, not fixed here. */
    puts("PASS captured red R, blue B, green uncertain, neutral whites W");
}
