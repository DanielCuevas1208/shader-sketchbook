#include "native_window.hpp"
#include "opengl_renderer.hpp"

#include "shader_sketchbook/lesson_catalog.hpp"
#include "shader_sketchbook/sdf_scene.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

#ifndef SKETCHBOOK_SHADER_DIR
#error "SKETCHBOOK_SHADER_DIR must point to the GLSL asset directory"
#endif

namespace {

std::string make_title(const shader_sketchbook::SceneSettings& settings, bool paused, std::size_t lesson_index,
                       const shader_sketchbook::ShaderLesson& lesson, const std::string& renderer_status) {
    std::ostringstream title;
    title << std::fixed << std::setprecision(2)
          << "Shader Sketchbook  |  lesson " << (lesson_index + 1) << "/" << shader_sketchbook::lesson_count()
          << "  " << lesson.title << "  |  blend " << settings.shape_blend
          << "  radius " << settings.sphere_radius
          << "  repeat " << settings.repeat
          << "  light " << settings.light_energy;
    if (paused) {
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

    SceneSettings settings{};
    bool paused = false;
    std::size_t active_lesson = 0;
    const std::filesystem::path shader_directory(SKETCHBOOK_SHADER_DIR);

    auto select_lesson = [&](std::size_t index) {
        const std::size_t candidate_index = index % lesson_count();
        const ShaderLesson& candidate = lesson_at(candidate_index);
        if (renderer.load_fragment_shader(shader_directory / candidate.fragment_file)) {
            active_lesson = candidate_index;
        }
    };

    window.set_key_handler([&](unsigned int key) {
        switch (key) {
        case 'A': settings.shape_blend = std::max(0.0F, settings.shape_blend - 0.03F); break;
        case 'D': settings.shape_blend = std::min(1.0F, settings.shape_blend + 0.03F); break;
        case 'W': settings.sphere_radius = std::min(1.20F, settings.sphere_radius + 0.03F); break;
        case 'S': settings.sphere_radius = std::max(0.25F, settings.sphere_radius - 0.03F); break;
        case 'Q': settings.repeat = std::min(5.0F, settings.repeat + 0.10F); break;
        case 'E': settings.repeat = std::max(0.80F, settings.repeat - 0.10F); break;
        case 'Z': settings.light_energy = std::max(0.0F, settings.light_energy - 0.05F); break;
        case 'X': settings.light_energy = std::min(2.5F, settings.light_energy + 0.05F); break;
        case 'N': select_lesson(next_lesson(active_lesson, 1)); break;
        case 'P': select_lesson(next_lesson(active_lesson, -1)); break;
        case VK_RIGHT: select_lesson(next_lesson(active_lesson, 1)); break;
        case VK_LEFT: select_lesson(next_lesson(active_lesson, -1)); break;
        case '1': select_lesson(0); break;
        case '2': select_lesson(1); break;
        case '3': select_lesson(2); break;
        case 'R': renderer.reload_shaders(); break;
        case VK_SPACE: paused = !paused; break;
        case VK_ESCAPE: window.close(); break;
        default: break;
        }
    });

    const auto start = std::chrono::steady_clock::now();
    auto last_title = start;
    while (!window.should_close()) {
        window.pump_messages();
        renderer.resize(window.width(), window.height());
        const auto now = std::chrono::steady_clock::now();
        const float elapsed = std::chrono::duration<float>(now - start).count();
        renderer.render(settings, paused ? 0.0F : elapsed);
        renderer.present();
        if (now - last_title > std::chrono::milliseconds(250)) {
            window.set_title(make_title(settings, paused, active_lesson, lesson_at(active_lesson), renderer.status()));
            last_title = now;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}


