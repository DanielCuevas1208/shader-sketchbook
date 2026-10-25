#include "native_window.hpp"
#include "opengl_renderer.hpp"

#include "shader_sketchbook/lesson_catalog.hpp"
#include "shader_sketchbook/lesson_session.hpp"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

#ifndef SKETCHBOOK_SHADER_DIR
#error "SKETCHBOOK_SHADER_DIR must point to the GLSL asset directory"
#endif

namespace {

std::string make_title(const shader_sketchbook::LessonSession& session,
                       const shader_sketchbook::LessonInfo& lesson,
                       std::size_t lesson_index,
                       const std::string& renderer_status) {
    std::ostringstream title;
    const shader_sketchbook::SceneSettings& settings = session.settings();
    title << "Shader Sketchbook  |  lesson " << (lesson_index + 1) << "/" << shader_sketchbook::lesson_count()
          << "  " << lesson.title
          << "  |  blend " << std::fixed << std::setprecision(2) << settings.shape_blend
          << "  radius " << settings.sphere_radius
          << "  repeat " << settings.repeat
          << "  light " << settings.light_energy;
    if (session.paused()) {
        title << "  |  PAUSED";
    }
    title << "  |  " << renderer_status;
    return title.str();
}

}  // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    using namespace shader_sketchbook;
    using namespace shader_sketchbook::demo;

    NativeWindow window;
    if (!window.create(1280, 720, "Shader Sketchbook")) {
        return 1;
    }

    OpenGLRenderer renderer;
    if (!renderer.initialize(window.handle(), std::filesystem::path(SKETCHBOOK_SHADER_DIR))) {
        window.set_title("Shader Sketchbook | OpenGL initialization failed");
        return 1;
    }
    renderer.resize(window.width(), window.height());

    LessonSession session;
    if (!session.load_shader_assets(std::filesystem::path(SKETCHBOOK_SHADER_DIR))) {
        window.set_title(std::string("Shader Sketchbook | ") + session.status());
        return 1;
    }
    window.set_key_handler([&](unsigned int key) {
        switch (key) {
        case 'A': session.handle(LessonCommand::DecreaseBlend); break;
        case 'D': session.handle(LessonCommand::IncreaseBlend); break;
        case 'W': session.handle(LessonCommand::IncreaseRadius); break;
        case 'S': session.handle(LessonCommand::DecreaseRadius); break;
        case 'Q': session.handle(LessonCommand::IncreaseRepeat); break;
        case 'E': session.handle(LessonCommand::DecreaseRepeat); break;
        case 'Z': session.handle(LessonCommand::DecreaseLight); break;
        case 'X': session.handle(LessonCommand::IncreaseLight); break;
        case VK_LEFT:
        case VK_OEM_COMMA:
            session.handle(LessonCommand::PreviousLesson);
            break;
        case VK_RIGHT:
        case VK_OEM_PERIOD:
            session.handle(LessonCommand::NextLesson);
            break;
        case 'R': session.handle(LessonCommand::ReloadShaders); break;
        case 'C': session.handle(LessonCommand::CaptureFrame); break;
        case VK_SPACE: session.handle(LessonCommand::TogglePause); break;
        case VK_ESCAPE:
            session.handle(LessonCommand::Close);
            window.close();
            break;
        default: break;
        }
    });

    const auto start = std::chrono::steady_clock::now();
    auto last_title = start;
    while (!window.should_close() && !session.should_close()) {
        window.pump_messages();
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
                std::filesystem::path("captures") / (std::string(renderer.current_lesson().id) + ".ppm");
            renderer.capture_frame(capture_path);
        }
        renderer.present();
        if (now - last_title > std::chrono::milliseconds(250)) {
            window.set_title(make_title(session, renderer.current_lesson(), renderer.lesson_index(), renderer.status()));
            last_title = now;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}
