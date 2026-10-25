#include "shader_program.hpp"

#include "shader_sketchbook/image.hpp"
#include "shader_sketchbook/lesson_catalog.hpp"
#include "shader_sketchbook/lesson_session.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#ifndef SKETCHBOOK_SHADER_DIR
#error "SKETCHBOOK_SHADER_DIR must point to the GLSL asset directory"
#endif

namespace {

class X11Window {
public:
    ~X11Window() {
        if (display_ != nullptr) {
            glXMakeCurrent(display_, None, nullptr);
            if (context_ != nullptr) {
                glXDestroyContext(display_, context_);
            }
            if (window_ != 0) {
                XDestroyWindow(display_, window_);
            }
            if (colormap_ != 0) {
                XFreeColormap(display_, colormap_);
            }
            XCloseDisplay(display_);
        }
    }

    bool create(int width, int height, const char* title) {
        display_ = XOpenDisplay(nullptr);
        if (display_ == nullptr) {
            return false;
        }

        const int visual_attributes[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, None};
        XVisualInfo* visual = glXChooseVisual(display_, DefaultScreen(display_), visual_attributes);
        if (visual == nullptr) {
            return false;
        }

        colormap_ = XCreateColormap(display_, RootWindow(display_, visual->screen), visual->visual, AllocNone);
        XSetWindowAttributes attributes{};
        attributes.colormap = colormap_;
        attributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
        window_ = XCreateWindow(display_,
                                RootWindow(display_, visual->screen),
                                0,
                                0,
                                static_cast<unsigned int>(width),
                                static_cast<unsigned int>(height),
                                0,
                                visual->depth,
                                InputOutput,
                                visual->visual,
                                CWColormap | CWEventMask,
                                &attributes);
        if (window_ == 0) {
            XFree(visual);
            return false;
        }

        XStoreName(display_, window_, title);
        delete_message_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display_, window_, &delete_message_, 1);
        context_ = glXCreateContext(display_, visual, nullptr, True);
        XFree(visual);
        if (context_ == nullptr || glXMakeCurrent(display_, window_, context_) == False) {
            return false;
        }

        width_ = width;
        height_ = height;
        XMapWindow(display_, window_);
        XFlush(display_);
        return true;
    }

    void pump(shader_sketchbook::LessonSession& session) {
        while (display_ != nullptr && XPending(display_) > 0) {
            XEvent event{};
            XNextEvent(display_, &event);
            if (event.type == ConfigureNotify) {
                width_ = event.xconfigure.width;
                height_ = event.xconfigure.height;
            } else if (event.type == DestroyNotify) {
                session.handle(shader_sketchbook::LessonCommand::Close);
            } else if (event.type == ClientMessage &&
                       static_cast<Atom>(event.xclient.data.l[0]) == delete_message_) {
                session.handle(shader_sketchbook::LessonCommand::Close);
            } else if (event.type == KeyPress) {
                handle_key(XLookupKeysym(&event.xkey, 0), session);
            }
        }
    }

    void set_title(const std::string& title) {
        if (display_ != nullptr && window_ != 0) {
            XStoreName(display_, window_, title.c_str());
            XFlush(display_);
        }
    }

    void present() noexcept {
        if (display_ != nullptr && window_ != 0) {
            glXSwapBuffers(display_, window_);
        }
    }

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }

private:
    static void handle_key(KeySym key, shader_sketchbook::LessonSession& session) {
        using shader_sketchbook::LessonCommand;
        LessonCommand command = LessonCommand::Close;
        bool handled = true;
        switch (key) {
        case XK_Left:
        case XK_comma: command = LessonCommand::PreviousLesson; break;
        case XK_Right:
        case XK_period: command = LessonCommand::NextLesson; break;
        case XK_a:
        case XK_A: command = LessonCommand::DecreaseBlend; break;
        case XK_d:
        case XK_D: command = LessonCommand::IncreaseBlend; break;
        case XK_s:
        case XK_S: command = LessonCommand::DecreaseRadius; break;
        case XK_w:
        case XK_W: command = LessonCommand::IncreaseRadius; break;
        case XK_e:
        case XK_E: command = LessonCommand::DecreaseRepeat; break;
        case XK_q:
        case XK_Q: command = LessonCommand::IncreaseRepeat; break;
        case XK_z:
        case XK_Z: command = LessonCommand::DecreaseLight; break;
        case XK_x:
        case XK_X: command = LessonCommand::IncreaseLight; break;
        case XK_r:
        case XK_R: command = LessonCommand::ReloadShaders; break;
        case XK_c:
        case XK_C: command = LessonCommand::CaptureFrame; break;
        case XK_space: command = LessonCommand::TogglePause; break;
        case XK_Escape: command = LessonCommand::Close; break;
        default: handled = false; break;
        }
        if (handled) {
            session.handle(command);
        }
    }

    Display* display_ = nullptr;
    ::Window window_ = 0;
    Colormap colormap_ = 0;
    Atom delete_message_ = None;
    GLXContext context_ = nullptr;
    int width_ = 1;
    int height_ = 1;
};

class X11Renderer {
public:
    ~X11Renderer() { shaders_.destroy(); }

    bool initialize(const std::filesystem::path& shader_directory, int width, int height) {
        if (!api_.load()) {
            status_ = "OpenGL shader functions are unavailable";
            return false;
        }
        lesson_directory_ = shader_directory / "lessons";
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.015F, 0.02F, 0.05F, 1.0F);
        resize(width, height);
        return select_lesson(0);
    }

    void resize(int width, int height) noexcept {
        width_ = std::max(width, 1);
        height_ = std::max(height, 1);
        glViewport(0, 0, width_, height_);
    }

    bool select_lesson(std::size_t index) {
        const std::size_t previous_index = lesson_index_;
        lesson_index_ = index % shader_sketchbook::lesson_count();
        const shader_sketchbook::LessonInfo& lesson = shader_sketchbook::lesson_at(lesson_index_);
        if (!shaders_.load(lesson_directory_ / "fullscreen.vert", lesson_directory_ / lesson.fragment_shader)) {
            status_ = shaders_.last_error();
            lesson_index_ = previous_index;
            return false;
        }
        status_ = std::string("lesson: ") + lesson.title;
        return true;
    }

    bool reload_shaders() {
        const bool loaded = shaders_.force_reload();
        status_ = loaded ? std::string("lesson: ") + shader_sketchbook::lesson_at(lesson_index_).title + ", reloaded"
                         : shaders_.last_error();
        return loaded;
    }

    void render(const shader_sketchbook::SceneSettings& settings, float time_seconds) {
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

    bool capture_frame(const std::filesystem::path& path) {
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

            shader_sketchbook::Image image(width_, height_);
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

    std::size_t lesson_index() const noexcept { return lesson_index_; }
    const std::string& status() const noexcept { return status_; }

private:
    shader_sketchbook::demo::GlApi api_{};
    shader_sketchbook::demo::ShaderProgram shaders_{api_};
    std::filesystem::path lesson_directory_;
    std::size_t lesson_index_ = 0;
    int width_ = 1;
    int height_ = 1;
    std::string status_ = "starting";
};

std::string make_title(const shader_sketchbook::LessonSession& session,
                       const shader_sketchbook::LessonInfo& lesson,
                       const std::string& renderer_status) {
    std::ostringstream title;
    const shader_sketchbook::SceneSettings& settings = session.settings();
    title << "Shader Sketchbook | " << lesson.title << " | blend " << std::fixed << std::setprecision(2)
          << settings.shape_blend << " radius " << settings.sphere_radius << " repeat " << settings.repeat
          << " light " << settings.light_energy;
    if (session.paused()) {
        title << " | PAUSED";
    }
    title << " | " << renderer_status;
    return title.str();
}

}  // namespace

int main() {
    using namespace shader_sketchbook;

    const std::filesystem::path shader_directory(SKETCHBOOK_SHADER_DIR);
    LessonSession session;
    if (!session.load_shader_assets(shader_directory)) {
        std::cerr << "shader_sketchbook_x11: " << session.status() << '\n';
        return 1;
    }

    X11Window window;
    if (!window.create(1280, 720, "Shader Sketchbook")) {
        std::cerr << "shader_sketchbook_x11: could not create an X11/OpenGL window\n";
        return 1;
    }

    X11Renderer renderer;
    if (!renderer.initialize(shader_directory, window.width(), window.height())) {
        std::cerr << "shader_sketchbook_x11: " << renderer.status() << '\n';
        return 1;
    }

    const auto start = std::chrono::steady_clock::now();
    auto last_title = start;
    while (!session.should_close()) {
        window.pump(session);
        renderer.resize(window.width(), window.height());
        if (renderer.lesson_index() != session.lesson_index()) {
            if (!renderer.select_lesson(session.lesson_index())) {
                session.handle(LessonCommand::Close);
            }
            session.take_reload_request();
        } else if (session.take_reload_request()) {
            renderer.reload_shaders();
        }

        const auto now = std::chrono::steady_clock::now();
        const float elapsed = std::chrono::duration<float>(now - start).count();
        renderer.render(session.settings(), session.paused() ? 0.0F : elapsed);
        if (session.take_capture_request()) {
            const std::filesystem::path capture_path =
                std::filesystem::path("captures") / (std::string(lesson_at(session.lesson_index()).id) + ".ppm");
            renderer.capture_frame(capture_path);
        }
        window.present();
        if (now - last_title > std::chrono::milliseconds(250)) {
            window.set_title(make_title(session, lesson_at(session.lesson_index()), renderer.status()));
            last_title = now;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}
