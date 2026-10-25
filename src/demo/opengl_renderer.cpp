#include "opengl_renderer.hpp"

#include <cstdint>
#include <exception>
#include <system_error>
#include <vector>

namespace shader_sketchbook::demo {

OpenGLRenderer::~OpenGLRenderer() {
    shutdown();
}

bool OpenGLRenderer::create_context(HWND window) {
    device_context_ = GetDC(window);
    if (device_context_ == nullptr) {
        status_ = "GetDC failed";
        return false;
    }

    PIXELFORMATDESCRIPTOR descriptor{};
    descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    descriptor.nVersion = 1;
    descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    descriptor.iPixelType = PFD_TYPE_RGBA;
    descriptor.cColorBits = 32;
    descriptor.cDepthBits = 24;
    descriptor.cStencilBits = 8;

    const int format = ChoosePixelFormat(device_context_, &descriptor);
    if (format == 0 || SetPixelFormat(device_context_, format, &descriptor) == FALSE) {
        status_ = "pixel format setup failed";
        return false;
    }

    context_ = wglCreateContext(device_context_);
    if (context_ == nullptr || wglMakeCurrent(device_context_, context_) == FALSE) {
        status_ = "OpenGL context setup failed";
        return false;
    }
    if (!api_.load()) {
        status_ = "OpenGL shader functions are unavailable";
        return false;
    }
    return true;
}

bool OpenGLRenderer::load_current_lesson() {
    const LessonInfo& lesson = lesson_at(lesson_index_);
    const bool loaded = shaders_.load(lesson_directory_ / "fullscreen.vert",
                                      lesson_directory_ / lesson.fragment_shader);
    if (!loaded) {
        status_ = shaders_.last_error();
        return false;
    }
    status_ = std::string("lesson: ") + lesson.title;
    return true;
}

bool OpenGLRenderer::initialize(HWND window, const std::filesystem::path& shader_directory) {
    window_ = window;
    lesson_directory_ = shader_directory / "lessons";
    if (!create_context(window_)) {
        return false;
    }

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.015F, 0.02F, 0.05F, 1.0F);
    resize(1, 1);

    if (!load_current_lesson()) {
        return false;
    }
    return true;
}

void OpenGLRenderer::shutdown() noexcept {
    if (context_ != nullptr) {
        wglMakeCurrent(device_context_, context_);
        shaders_.destroy();
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(context_);
        context_ = nullptr;
    }
    if (device_context_ != nullptr && window_ != nullptr) {
        ReleaseDC(window_, device_context_);
        device_context_ = nullptr;
    }
}

void OpenGLRenderer::resize(int width, int height) noexcept {
    width_ = width > 0 ? width : 1;
    height_ = height > 0 ? height : 1;
    if (context_ != nullptr) {
        glViewport(0, 0, width_, height_);
    }
}

void OpenGLRenderer::render(const SceneSettings& settings, float time_seconds) {
    if (context_ == nullptr) {
        return;
    }
    if (shaders_.reload_if_changed()) {
        status_ = std::string("lesson: ") + current_lesson().title + ", reloaded";
    }

    glClear(GL_COLOR_BUFFER_BIT);
    shaders_.bind();
    const GLint resolution = shaders_.uniform("u_resolution");
    const GLint time = shaders_.uniform("u_time");
    const GLint blend = shaders_.uniform("u_shape_blend");
    const GLint radius = shaders_.uniform("u_sphere_radius");
    const GLint repeat = shaders_.uniform("u_repeat");
    const GLint energy = shaders_.uniform("u_light_energy");
    if (resolution >= 0) {
        api_.uniform_2f(resolution, static_cast<GLfloat>(width_), static_cast<GLfloat>(height_));
    }
    if (time >= 0) {
        api_.uniform_1f(time, time_seconds);
    }
    if (blend >= 0) {
        api_.uniform_1f(blend, settings.shape_blend);
    }
    if (radius >= 0) {
        api_.uniform_1f(radius, settings.sphere_radius);
    }
    if (repeat >= 0) {
        api_.uniform_1f(repeat, settings.repeat);
    }
    if (energy >= 0) {
        api_.uniform_1f(energy, settings.light_energy);
    }

    glBegin(GL_TRIANGLES);
    glVertex2f(-1.0F, -1.0F);
    glVertex2f(3.0F, -1.0F);
    glVertex2f(-1.0F, 3.0F);
    glEnd();
}

bool OpenGLRenderer::capture_frame(const std::filesystem::path& path) {
    if (context_ == nullptr) {
        status_ = "capture unavailable";
        return false;
    }

    try {
        const std::filesystem::path parent = path.parent_path();
        if (!parent.empty()) {
            std::error_code error;
            std::filesystem::create_directories(parent, error);
            if (error) {
                status_ = "capture directory failed: " + error.message();
                return false;
            }
        }

        const std::size_t pixel_count = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
        std::vector<std::uint8_t> rgba(pixel_count * 4U);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
        if (glGetError() != GL_NO_ERROR) {
            status_ = "capture readback failed";
            return false;
        }

        Image image(width_, height_);
        for (int y = 0; y < height_; ++y) {
            const int source_y = height_ - 1 - y;
            for (int x = 0; x < width_; ++x) {
                const std::size_t source =
                    (static_cast<std::size_t>(source_y) * static_cast<std::size_t>(width_) +
                     static_cast<std::size_t>(x)) * 4U;
                image.at(x, y) = {rgba[source], rgba[source + 1U], rgba[source + 2U], 255};
            }
        }
        image.save_ppm(path);
        status_ = "capture: " + path.string();
        return true;
    } catch (const std::exception& error) {
        status_ = "capture failed: " + std::string(error.what());
        return false;
    }
}

bool OpenGLRenderer::reload_shaders() {
    const bool loaded = shaders_.force_reload();
    status_ = loaded ? std::string("lesson: ") + current_lesson().title + ", reloaded" : shaders_.last_error();
    return loaded;
}

bool OpenGLRenderer::select_lesson(std::size_t index) {
    const std::size_t previous_index = lesson_index_;
    lesson_index_ = index % lesson_count();
    if (load_current_lesson()) {
        return true;
    }
    lesson_index_ = previous_index;
    return false;
}

bool OpenGLRenderer::cycle_lesson(int delta) {
    return select_lesson(wrap_lesson_index(static_cast<int>(lesson_index_) + delta));
}

void OpenGLRenderer::present() noexcept {
    if (device_context_ != nullptr) {
        SwapBuffers(device_context_);
    }
}

}  // namespace shader_sketchbook::demo
