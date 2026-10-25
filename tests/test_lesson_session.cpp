#include "shader_sketchbook/lesson_session.hpp"

#include "shader_sketchbook/lesson_catalog.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#ifndef SKETCHBOOK_SHADER_DIR
#error "SKETCHBOOK_SHADER_DIR must point to the shader asset directory"
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

    LessonSession session;
    require(session.load_shader_assets(std::filesystem::path(SKETCHBOOK_SHADER_DIR)),
            "all catalog lessons load without a display");
    require(session.lesson_index() == 0, "session starts at the first lesson");

    require(session.handle(LessonCommand::PreviousLesson), "previous lesson command is handled");
    require(session.lesson_index() == lesson_count() - 1, "previous lesson wraps at the catalog boundary");
    require(session.take_reload_request(), "lesson changes request a shader reload");
    require(!session.take_reload_request(), "reload requests are consumed once");

    const SceneSettings defaults = session.settings();
    require(session.handle(LessonCommand::IncreaseBlend), "blend command is handled");
    require(session.settings().shape_blend > defaults.shape_blend, "blend control changes session state");
    require(session.handle(LessonCommand::TogglePause), "pause command is handled");
    require(session.paused(), "pause command toggles paused state");
    require(session.handle(LessonCommand::ReloadShaders), "reload command is handled");
    require(session.take_reload_request(), "reload command reaches the backend");
    require(session.handle(LessonCommand::CaptureFrame), "capture command is handled");
    require(session.take_capture_request(), "capture command reaches the backend");

    LessonSession missing;
    require(!missing.load_shader_assets(std::filesystem::path(SKETCHBOOK_SHADER_DIR) / "missing"),
            "missing shader roots fail clearly");
    require(missing.status().find("missing shader") == 0, "missing assets provide a useful status");

    std::cout << "lesson_session: PASS\n";
    std::cout << "lesson_count: " << lesson_count() << '\n';
    return EXIT_SUCCESS;
}
