/*
 * SPDX-FileCopyrightText: Copyright 2024, 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAL_CAMERA_H
#define HAL_CAMERA_H

#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)

#include <stdint.h>
#include <stdbool.h>

/* Saved tablecal white gains, derived from the accepted profile's white
 * reference.  They are deliberately applied only to display/model views;
 * the CPU reader continues to receive raw camera RGB and owns its existing
 * calibration correction. */
#define CUBE_TABLECAL_WHITE_GAIN_R 6.03101015f
#define CUBE_TABLECAL_WHITE_GAIN_G 4.33846188f
#define CUBE_TABLECAL_WHITE_GAIN_B 5.17589474f

/**< Camera device's operating modes */
typedef enum hal_camera_mode_ {
    HAL_CAMERA_MODE_SINGLE_FRAME = 0,
    HAL_CAMERA_MODE_CONTINUOUS,
    HAL_CAMERA_MODE_INVALID
} hal_cam_mode;

/**< Image colour formats supported by this interface */
typedef enum hal_camera_colour_format_ {
    HAL_CAMERA_COLOUR_FORMAT_RGB888 = 0,
    HAL_CAMERA_COLOUR_FORMAT_RGB565,
    HAL_CAMERA_COLOUR_FORMAT_INVALID
} hal_cam_clr_format;

/**< Camera device status */
typedef enum hal_camera_status_ {
    HAL_CAMERA_STATUS_ERROR = -1,
    HAL_CAMERA_STATUS_STOPPED = 0,
    HAL_CAMERA_STATUS_RUNNING,
    HAL_CAMERA_STATUS_INVALID
} hal_cam_status;

/**
* @brief  Initialises the camera interface.
* @return true if successful, false otherwise.
*/
bool hal_camera_init(void);

/**
 *  @brief Configure camera device.
 *  @param[in]  width           Image width
 *  @param[in]  height          Image height
 *  @param[in]  mode            Camera mode
 *  @param[in]  colour_format   Colour format
 *  @return     true if successful, false otherwise.
 */
bool hal_camera_configure(uint32_t width,
                          uint32_t height,
                          hal_cam_mode mode,
                          hal_cam_clr_format colour_format);

/**
 *  @brief Sets the image buffer location.
 *  @param[in]  buffer      Buffer to be used for populating image data.
 *  @param[in]  size        Size of the buffer provided.
 *  @return     true if successful, false otherwise.
 */
bool hal_camera_set_buffer(uint8_t* buffer, const uint32_t size);

/** Returns the most recent full-panel RGB888 preview in LVGL B,G,R byte order. */
const uint8_t *hal_camera_get_preview_frame(void);

/**
 *  @brief Starts capturing data from camera device.
 *  @return     true if successful, false otherwise.
 */
bool hal_camera_start(void);

/**
 *  @brief Gets pointer to the captured frame.
 *  @param[out] size Bytes occupied by the current frame.
 *  @return     Pointer to 8-bit unsigned data.
 */
const uint8_t* hal_camera_get_captured_frame(uint32_t* size);

/* Read nine cells from the selected ML-frame face ROI.  The implementation
 * resamples a private BGR tile and reuses the validated cube calibration;
 * the caller's model frame is never modified. */
bool hal_camera_cube_read_selected_roi(const uint8_t* rgb, uint32_t frame_width,
                                       uint32_t frame_height, int32_t x0,
                                       int32_t y0, int32_t width, int32_t height,
                                       char letters[10]);

/* Human-readable CAL/READ state maintained by the accepted any-button path. */
const char* hal_camera_cube_status(void);
bool hal_camera_cube_is_read_mode(void);

/**
 * @brief  Stops the camera device.
 * @return true if successful, false otherwise.
 */
bool hal_camera_stop(void);

/**
 * @brief  Gets camera device status.
 * @return Status represented as the status enumeration.
 */
hal_cam_status hal_camera_get_status(void);

/**
 * @brief  Releases the camera device.
 */
void hal_camera_release(void);

/**
 * @brief  Gets the human friendly name of the camera device.
 * @return Name expressed as a C string.
 */
const char* hal_camera_get_device_name(void);

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)
#endif /* HAL_CAMERA_H */
