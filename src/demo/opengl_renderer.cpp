#include "opengl_renderer.hpp"

#include <iostream>

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

bool OpenGLRenderer::initialize(HWND window, const std::filesystem::path& shader_directory) {
    window_ = window;
    shader_directory_ = shader_directory;
    if (!create_context(window_)) {
        return false;
    }

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.015F, 0.02F, 0.05F, 1.0F);
    resize(1, 1);

    const bool loaded = shaders_.load(shader_directory_ / "scene.vert", shader_directory_ / "scene.frag");
    if (!loaded) {
        status_ = shaders_.last_error();
        return false;
    }
    status_ = "live shader";
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
        status_ = "live shader, reloaded";
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

bool OpenGLRenderer::reload_shaders() {
    const bool loaded = shaders_.force_reload();
    status_ = loaded ? "live shader, reloaded" : shaders_.last_error();
    return loaded;
}

void OpenGLRenderer::present() noexcept {
    if (device_context_ != nullptr) {
        SwapBuffers(device_context_);
    }
}

}  // namespace shader_sketchbook::demo
