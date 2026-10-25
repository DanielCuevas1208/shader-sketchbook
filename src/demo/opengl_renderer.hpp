#pragma once

#include "shader_program.hpp"

#include "shader_sketchbook/lesson_catalog.hpp"
#include "shader_sketchbook/sdf_scene.hpp"

#include <cstddef>
#include <filesystem>
#include <string>

namespace shader_sketchbook::demo {

class OpenGLRenderer {
public:
    OpenGLRenderer() : shaders_(api_) {}
    ~OpenGLRenderer();

    bool initialize(HWND window, const std::filesystem::path& shader_directory);
    void shutdown() noexcept;
    void resize(int width, int height) noexcept;
    void render(const SceneSettings& settings, float time_seconds);
    bool capture_frame(const std::filesystem::path& path);
    bool reload_shaders();
    bool select_lesson(std::size_t index);
    bool cycle_lesson(int delta);
    void present() noexcept;

    std::size_t lesson_index() const noexcept { return lesson_index_; }
    const LessonInfo& current_lesson() const noexcept { return lesson_at(lesson_index_); }
    const std::string& status() const noexcept { return status_; }

private:
    bool create_context(HWND window);
    bool load_current_lesson();

    HWND window_ = nullptr;
    HDC device_context_ = nullptr;
    HGLRC context_ = nullptr;
    int width_ = 1;
    int height_ = 1;
    GlApi api_{};
    ShaderProgram shaders_;
    std::filesystem::path lesson_directory_;
    std::size_t lesson_index_ = 0;
    std::string status_ = "starting";
};

}  // namespace shader_sketchbook::demo
