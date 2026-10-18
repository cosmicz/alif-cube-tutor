#include "CubeFaceMultiSelect.hpp"

#include <array>
#include <cassert>

int main()
{
    constexpr size_t channels = 5, candidates = 756;
    std::array<float, channels * candidates> output{};
    const auto set = [&](size_t c, float x, float y, float w, float h, float conf) {
        output[0 * candidates + c] = x; output[1 * candidates + c] = y;
        output[2 * candidates + c] = w; output[3 * candidates + c] = h; output[4 * candidates + c] = conf;
    };
    const auto valueAt = [&](size_t index) { return output[index]; };
    size_t selected[3];
    /* Two faces apart, a near-duplicate of the first, one bright sticker, one weak box. */
    set(10, 0.30f, 0.50f, 0.30f, 0.30f, 0.60f);   /* face A, area 0.09 */
    set(11, 0.31f, 0.50f, 0.30f, 0.30f, 0.55f);   /* duplicate of A, IoU ~0.94 */
    set(20, 0.75f, 0.50f, 0.40f, 0.40f, 0.40f);   /* face B, area 0.16 (largest) */
    set(30, 0.50f, 0.20f, 0.17f, 0.16f, 0.999f);  /* sticker, area 0.027 */
    set(40, 0.50f, 0.80f, 0.30f, 0.30f, 0.10f);   /* below threshold */
    size_t n = alif::app::object_detection::SelectCubeFaces(valueAt, candidates, 0.25f, 0.04f, 0.5f, 3, selected);
    assert(n == 2);
    assert(selected[0] == 20 && selected[1] == 10);  /* largest first, duplicate and sticker gone */
    /* Four faces: at most three, largest first. */
    set(50, 0.20f, 0.20f, 0.25f, 0.25f, 0.50f);
    set(60, 0.80f, 0.85f, 0.20f, 0.20f, 0.50f);
    n = alif::app::object_detection::SelectCubeFaces(valueAt, candidates, 0.25f, 0.04f, 0.5f, 3, selected);
    assert(n == 3 && selected[0] == 20 && selected[1] == 10 && selected[2] == 50);
    /* Single-face scene unchanged: one result. */
    std::array<float, channels * candidates> single{};
    single[2 * candidates + 5] = 0.4f; single[3 * candidates + 5] = 0.4f; single[4 * candidates + 5] = 0.9f;
    const auto one = [&](size_t index) { return single[index]; };
    n = alif::app::object_detection::SelectCubeFaces(one, candidates, 0.25f, 0.04f, 0.5f, 3, selected);
    assert(n == 1 && selected[0] == 5);
    return 0;
}
