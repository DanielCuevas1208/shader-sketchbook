#include "shader_sketchbook/lesson_catalog.hpp"

#include <array>
#include <cstring>

namespace shader_sketchbook {
namespace {

constexpr std::array<LessonInfo, 5> kLessons{{
    {"composite_scene",
     "Composite Scene",
     "Blend a sphere, a box, repeated satellites, and a ground plane.",
     "composite_scene.frag"},
    {"smooth_union",
     "Smooth Union",
     "Morph two signed-distance primitives with a smooth minimum.",
     "smooth_union.frag"},
    {"domain_repeat",
     "Domain Repeat",
     "Tile space and render a lattice of small spheres.",
     "domain_repeat.frag"},
    {"ray_march",
     "Ray March",
     "March rays through a single field and shade the first hit.",
     "ray_march.frag"},
    {"normal_shading",
     "Normal Shading",
     "Estimate normals from the field and apply diffuse lighting.",
     "normal_shading.frag"},
}};

}  // namespace

std::size_t lesson_count() noexcept {
    return kLessons.size();
}

const LessonInfo& lesson_at(std::size_t index) noexcept {
    return kLessons[index % kLessons.size()];
}

int find_lesson_index(const char* id) noexcept {
    if (id == nullptr) {
        return -1;
    }
    for (std::size_t index = 0; index < kLessons.size(); ++index) {
        if (std::strcmp(kLessons[index].id, id) == 0) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

std::size_t wrap_lesson_index(int index) noexcept {
    const int count = static_cast<int>(kLessons.size());
    int wrapped = index % count;
    if (wrapped < 0) {
        wrapped += count;
    }
    return static_cast<std::size_t>(wrapped);
}

}  // namespace shader_sketchbook
