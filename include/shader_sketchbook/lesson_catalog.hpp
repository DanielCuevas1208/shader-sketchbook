#pragma once

#include <cstddef>
#include <string_view>

namespace shader_sketchbook {

struct ShaderLesson {
    std::string_view id;
    std::string_view title;
    std::string_view summary;
    std::string_view focus;
    std::string_view fragment_file;
};

std::size_t lesson_count() noexcept;
const ShaderLesson& lesson_at(std::size_t index) noexcept;
const ShaderLesson* find_lesson(std::string_view id) noexcept;
std::size_t next_lesson(std::size_t current, int direction) noexcept;

}  // namespace shader_sketchbook
