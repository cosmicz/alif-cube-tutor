/* SPDX-License-Identifier: Apache-2.0 */
#ifndef CUBE_FACE_MULTI_SELECT_HPP
#define CUBE_FACE_MULTI_SELECT_HPP

#include <algorithm>
#include <cstddef>

namespace alif::app::object_detection {

/* One to three cube faces per image (arc-fdo1, 2026-10-18).  valueAt reads the
 * channel-major YOLOv8 output [x, y, width, height, confidence] in normalised
 * units.  Candidates below confidenceThreshold or smaller than minArea (single
 * stickers) are dropped; greedy non-maximum suppression by confidence removes
 * overlaps above iouThreshold; the survivors are returned largest first, at
 * most maxFaces of them.  Returns how many indices were written to selected. */
template <typename ValueAt>
size_t SelectCubeFaces(ValueAt valueAt, size_t candidates, float confidenceThreshold, float minArea,
                       float iouThreshold, size_t maxFaces, size_t* selected)
{
    const auto confidence = [&](size_t c) { return valueAt(4 * candidates + c); };
    const auto width  = [&](size_t c) { return std::max(0.0f, valueAt(2 * candidates + c)); };
    const auto height = [&](size_t c) { return std::max(0.0f, valueAt(3 * candidates + c)); };
    const auto area   = [&](size_t c) { return width(c) * height(c); };
    const auto iou = [&](size_t a, size_t b) {
        const float ax0 = valueAt(a) - width(a) / 2, ax1 = valueAt(a) + width(a) / 2;
        const float ay0 = valueAt(candidates + a) - height(a) / 2, ay1 = valueAt(candidates + a) + height(a) / 2;
        const float bx0 = valueAt(b) - width(b) / 2, bx1 = valueAt(b) + width(b) / 2;
        const float by0 = valueAt(candidates + b) - height(b) / 2, by1 = valueAt(candidates + b) + height(b) / 2;
        const float iw = std::max(0.0f, std::min(ax1, bx1) - std::max(ax0, bx0));
        const float ih = std::max(0.0f, std::min(ay1, by1) - std::max(ay0, by0));
        const float inter = iw * ih;
        const float uni = area(a) + area(b) - inter;
        return uni > 0.0f ? inter / uni : 0.0f;
    };
    size_t survivors[16];
    size_t count = 0;
    bool done[756] = {};
    if (candidates > sizeof(done) / sizeof(done[0]) || maxFaces == 0 || !selected) {
        return 0;
    }
    for (;;) {
        size_t best = candidates;
        float bestConfidence = confidenceThreshold;
        for (size_t c = 0; c < candidates; ++c) {
            if (done[c]) continue;
            const float conf = confidence(c);
            if (conf <= confidenceThreshold || area(c) < minArea) { done[c] = true; continue; }
            if (conf > bestConfidence) { best = c; bestConfidence = conf; }
        }
        if (best == candidates) break;
        done[best] = true;
        bool suppressed = false;
        for (size_t i = 0; i < count; ++i) {
            if (iou(best, survivors[i]) > iouThreshold) { suppressed = true; break; }
        }
        if (suppressed) continue;
        survivors[count++] = best;
        if (count == sizeof(survivors) / sizeof(survivors[0])) break;
    }
    std::sort(survivors, survivors + count, [&](size_t a, size_t b) { return area(a) > area(b); });
    const size_t kept = std::min(count, maxFaces);
    for (size_t i = 0; i < kept; ++i) selected[i] = survivors[i];
    return kept;
}

} /* namespace alif::app::object_detection */

#endif /* CUBE_FACE_MULTI_SELECT_HPP */
