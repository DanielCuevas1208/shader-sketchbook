#pragma once

#include <cstddef>

namespace shader_sketchbook {

struct LessonInfo {
    const char* id;
    const char* title;
    const char* summary;
    const char* fragment_shader;
};

std::size_t lesson_count() noexcept;
const LessonInfo& lesson_at(std::size_t index) noexcept;
int find_lesson_index(const char* id) noexcept;
std::size_t wrap_lesson_index(int index) noexcept;

}  // namespace shader_sketchbook
