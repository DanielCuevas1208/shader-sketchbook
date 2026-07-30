#include "shader_sketchbook/lesson_catalog.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#ifndef SKETCHBOOK_SHADER_DIR
#error "SKETCHBOOK_SHADER_DIR must point to the lesson shader assets"
#endif

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace

int main() {
    using namespace shader_sketchbook;

    require(lesson_count() == 3, "the release exposes three shader lessons");
    for (std::size_t index = 0; index < lesson_count(); ++index) {
        const ShaderLesson& lesson = lesson_at(index);
        require(!lesson.id.empty(), "every lesson has an id");
        require(!lesson.title.empty(), "every lesson has a title");
        require(!lesson.fragment_file.empty(), "every lesson points to a fragment shader");
        require(find_lesson(lesson.id) == &lesson, "lesson ids resolve to their catalog entry");
        require(std::filesystem::exists(std::filesystem::path(SKETCHBOOK_SHADER_DIR) / lesson.fragment_file),
                "every lesson points to an existing fragment shader");
    }

    require(find_lesson("missing") == nullptr, "unknown lesson ids return no match");
    require(next_lesson(0, 1) == 1, "forward navigation advances one lesson");
    require(next_lesson(0, -1) == lesson_count() - 1, "backward navigation wraps at the first lesson");
    require(next_lesson(lesson_count() - 1, 1) == 0, "forward navigation wraps at the last lesson");
    require(lesson_at(lesson_count() + 1).id == lesson_at(1).id, "lesson lookup wraps safely");

    std::cout << "lesson_catalog: PASS\n";
    std::cout << "lessons: " << lesson_count() << '\n';
    return EXIT_SUCCESS;
}
