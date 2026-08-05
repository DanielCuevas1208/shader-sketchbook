#include "shader_sketchbook/lesson_catalog.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>

#ifndef SKETCHBOOK_LESSON_DIR
#error "SKETCHBOOK_LESSON_DIR must point to the lesson shader directory"
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

    require(lesson_count() >= 4, "the catalog exposes at least four lessons");

    std::set<std::string> ids;
    for (std::size_t index = 0; index < lesson_count(); ++index) {
        const LessonInfo& lesson = lesson_at(index);
        require(lesson.id != nullptr && lesson.id[0] != '\0', "each lesson has an id");
        require(lesson.title != nullptr && lesson.title[0] != '\0', "each lesson has a title");
        require(lesson.summary != nullptr && lesson.summary[0] != '\0', "each lesson has a summary");
        require(lesson.fragment_shader != nullptr && lesson.fragment_shader[0] != '\0',
                "each lesson names a fragment shader");
        require(ids.insert(lesson.id).second, "lesson ids are unique");
    }

    const std::filesystem::path lesson_dir(SKETCHBOOK_LESSON_DIR);
    require(std::filesystem::exists(lesson_dir / "fullscreen.vert"), "the shared vertex shader exists");
    for (std::size_t index = 0; index < lesson_count(); ++index) {
        const LessonInfo& lesson = lesson_at(index);
        const std::filesystem::path fragment = lesson_dir / lesson.fragment_shader;
        require(std::filesystem::exists(fragment), std::string("fragment shader exists: ") + lesson.fragment_shader);
    }

    require(find_lesson_index("composite_scene") == 0, "composite_scene is the first lesson");
    require(find_lesson_index("missing_lesson") == -1, "unknown ids return a negative index");
    require(wrap_lesson_index(-1) == lesson_count() - 1, "negative indices wrap backward");
    require(wrap_lesson_index(static_cast<int>(lesson_count())) == 0, "out-of-range indices wrap forward");

    std::cout << "lesson_catalog: PASS\n";
    std::cout << "lesson_count: " << lesson_count() << '\n';
    return EXIT_SUCCESS;
}
