#ifndef CUBE_CAMERA_MAPPING_H
#define CUBE_CAMERA_MAPPING_H

#include <stdint.h>

/* Inverse map: sensor -> clockwise quarter-turn -> aspect-preserving central
 * crop -> one horizontal mirror in the logical LCD view. UI is not rotated
 * here; LVGL still owns ROTATE_DISPLAY=90 for the complete composition. */
static inline uint32_t cube_camera_source_index(uint32_t width, uint32_t height,
                                               uint32_t output_width,
                                               uint32_t output_height,
                                               uint32_t x, uint32_t y)
{
    const uint32_t rotated_width = height, rotated_height = width;
    const uint32_t crop_width = rotated_width * output_height > rotated_height * output_width
        ? rotated_height * output_width / output_height : rotated_width;
    const uint32_t crop_height = rotated_width * output_height > rotated_height * output_width
        ? rotated_height : rotated_width * output_height / output_width;
    const uint32_t rx = (rotated_width - crop_width) / 2 + crop_width - 1
        - x * crop_width / output_width;
    const uint32_t ry = (rotated_height - crop_height) / 2
        + y * crop_height / output_height;
    /* Clockwise output (rx,ry) comes from sensor (ry,height-1-rx). */
    return (height - 1 - rx) * width + ry;
}

#endif
