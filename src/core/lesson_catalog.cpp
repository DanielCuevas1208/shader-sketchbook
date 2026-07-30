#include "shader_sketchbook/lesson_catalog.hpp"

#include <array>

namespace shader_sketchbook {
namespace {

constexpr std::array<ShaderLesson, 3> kLessons{{
    {"sdf-fusion", "SDF Fusion", "Compose primitives into one ray-marched scene.", "smooth unions and distance fields", "scene.frag"},
    {"normals-lighting", "Normals and Lighting", "Turn a distance field into a responsive lit surface.", "finite differences and shading", "lessons/normals.frag"},
    {"space-repetition", "Space Repetition", "Fold space to build a field of repeated forms.", "modulo repetition and composition", "lessons/repetition.frag"},
}};

}  // namespace

std::size_t lesson_count() noexcept {
    return kLessons.size();
}

const ShaderLesson& lesson_at(std::size_t index) noexcept {
    return kLessons[index % kLessons.size()];
}

const ShaderLesson* find_lesson(std::string_view id) noexcept {
    for (const ShaderLesson& lesson : kLessons) {
        if (lesson.id == id) {
            return &lesson;
        }
    }
    return nullptr;
}

std::size_t next_lesson(std::size_t current, int direction) noexcept {
    const std::size_t count = lesson_count();
    const std::size_t normalized = current % count;
    const int step = direction % static_cast<int>(count);
    if (step < 0) {
        return (normalized + count - static_cast<std::size_t>(-step)) % count;
    }
    return (normalized + static_cast<std::size_t>(step)) % count;
}

}  // namespace shader_sketchbook
