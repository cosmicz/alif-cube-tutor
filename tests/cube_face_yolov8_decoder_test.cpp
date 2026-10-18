#include "CubeFaceYoloV8.hpp"

#include <array>
#include <cassert>

int main()
{
    constexpr size_t channels = 5;
    constexpr size_t candidates = 756;
    std::array<float, channels * candidates> output{};
    const auto set = [&](size_t channel, size_t candidate, float value) {
        output[channel * candidates + candidate] = value;
    };
    /* A bright sticker may score higher than the complete cube face. */
    set(2, 200, 0.17f);
    set(3, 200, 0.16f);
    set(4, 200, 0.999f);
    set(2, 642, 0.48f);
    set(3, 642, 0.48f);
    set(4, 642, 0.326f);

    const auto valueAt = [&](size_t index) { return output[index]; };
    assert(alif::app::object_detection::SelectLargestFaceCandidate(
                   valueAt, candidates, 0.25f) == 642);
    assert(alif::app::object_detection::SelectLargestFaceCandidate(
                   valueAt, candidates, 1.0f) == candidates);
    return 0;
}
