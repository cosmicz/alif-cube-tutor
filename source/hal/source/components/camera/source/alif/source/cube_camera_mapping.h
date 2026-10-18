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

typedef struct { float x, y; } cube_camera_point;

/* Forward map for model coordinates produced from the unrotated/unmirrored
 * sensor crop. It is the continuous inverse of cube_camera_source_index:
 * clockwise source rotation, central landscape crop, then one mirror. */
static inline cube_camera_point cube_camera_model_to_preview(
    uint32_t sensor_width, uint32_t sensor_height, uint32_t preview_width,
    uint32_t preview_height, uint32_t model_width, uint32_t model_height,
    float model_x, float model_y)
{
    const float rotated_width = (float)sensor_height;
    const float rotated_height = (float)sensor_width;
    const float crop_width = rotated_width * preview_height > rotated_height * preview_width
        ? rotated_height * preview_width / preview_height : rotated_width;
    const float crop_height = rotated_width * preview_height > rotated_height * preview_width
        ? rotated_height : rotated_width * preview_height / preview_width;
    const float sensor_x = model_x * sensor_width / model_width;
    const float sensor_y = model_y * sensor_height / model_height;
    const float rx = sensor_height - 1.0f - sensor_y;
    const float ry = sensor_x;
    cube_camera_point point = {
        ((rotated_width - crop_width) / 2.0f + crop_width - 1.0f - rx) * preview_width / crop_width,
        (ry - (rotated_height - crop_height) / 2.0f) * preview_height / crop_height
    };
    return point;
}

#endif
