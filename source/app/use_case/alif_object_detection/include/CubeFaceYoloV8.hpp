/* SPDX-License-Identifier: Apache-2.0 */
#ifndef CUBE_FACE_YOLO_V8_HPP
#define CUBE_FACE_YOLO_V8_HPP

#include <algorithm>
#include <cstddef>

namespace alif::app::object_detection {

/* Prefer the largest plausible face over a high-confidence single sticker.
 * valueAt is a channel-major YOLOv8 [x, y, width, height, confidence] reader. */
template <typename ValueAt>
size_t SelectLargestFaceCandidate(ValueAt valueAt, size_t candidates, float confidenceThreshold)
{
    size_t selected = candidates;
    float selectedArea = 0.0f;
    float selectedConfidence = confidenceThreshold;
    for (size_t candidate = 0; candidate < candidates; ++candidate) {
        const float confidence = valueAt(4 * candidates + candidate);
        if (confidence <= confidenceThreshold) {
            continue;
        }
        const float width = std::max(0.0f, valueAt(2 * candidates + candidate));
        const float height = std::max(0.0f, valueAt(3 * candidates + candidate));
        const float area = width * height;
        if (area > selectedArea || (area == selectedArea && confidence > selectedConfidence)) {
            selected = candidate;
            selectedArea = area;
            selectedConfidence = confidence;
        }
    }
    return selected;
}

} /* namespace alif::app::object_detection */

#endif /* CUBE_FACE_YOLO_V8_HPP */
