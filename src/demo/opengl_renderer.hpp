#pragma once

#include "shader_program.hpp"

#include "shader_sketchbook/sdf_scene.hpp"

#include <filesystem>
#include <string>

namespace shader_sketchbook::demo {

class OpenGLRenderer {
public:
    OpenGLRenderer() : shaders_(api_) {}
    ~OpenGLRenderer();

    bool initialize(HWND window, const std::filesystem::path& shader_directory);
    bool load_fragment_shader(const std::filesystem::path& fragment_path);
    void shutdown() noexcept;
    void resize(int width, int height) noexcept;
    void render(const SceneSettings& settings, float time_seconds);
    bool reload_shaders();
    void present() noexcept;

    const std::string& status() const noexcept { return status_; }

private:
    bool create_context(HWND window);

    HWND window_ = nullptr;
    HDC device_context_ = nullptr;
    HGLRC context_ = nullptr;
    int width_ = 1;
    int height_ = 1;
    GlApi api_{};
    ShaderProgram shaders_;
    std::filesystem::path shader_directory_;
    std::string status_ = "starting";
};

}  // namespace shader_sketchbook::demo
