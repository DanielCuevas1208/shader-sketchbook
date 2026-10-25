#include "shader_sketchbook/lesson_session.hpp"

#include "shader_sketchbook/lesson_catalog.hpp"

#include <algorithm>
#include <system_error>

namespace shader_sketchbook {
namespace {

constexpr float kBlendStep = 0.03F;
constexpr float kRadiusStep = 0.03F;
constexpr float kRepeatStep = 0.10F;
constexpr float kLightStep = 0.05F;

bool existing_file(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path, error) && !error;
}

}  // namespace

bool LessonSession::load_shader_assets(const std::filesystem::path& shader_directory) {
    const std::filesystem::path lesson_directory = shader_directory / "lessons";
    if (!existing_file(lesson_directory / "fullscreen.vert")) {
        status_ = "missing shader: fullscreen.vert";
        return false;
    }

    for (std::size_t index = 0; index < lesson_count(); ++index) {
        const LessonInfo& lesson = lesson_at(index);
        if (!existing_file(lesson_directory / lesson.fragment_shader)) {
            status_ = std::string("missing shader: ") + lesson.fragment_shader;
            return false;
        }
    }

    status_ = std::string("lesson: ") + lesson_at(lesson_index_).title;
    return true;
}

bool LessonSession::handle(LessonCommand command) {
    switch (command) {
    case LessonCommand::PreviousLesson:
        lesson_index_ = wrap_lesson_index(static_cast<int>(lesson_index_) - 1);
        reload_requested_ = true;
        status_ = std::string("lesson: ") + lesson_at(lesson_index_).title;
        return true;
    case LessonCommand::NextLesson:
        lesson_index_ = wrap_lesson_index(static_cast<int>(lesson_index_) + 1);
        reload_requested_ = true;
        status_ = std::string("lesson: ") + lesson_at(lesson_index_).title;
        return true;
    case LessonCommand::DecreaseBlend:
        settings_.shape_blend = std::max(0.0F, settings_.shape_blend - kBlendStep);
        return true;
    case LessonCommand::IncreaseBlend:
        settings_.shape_blend = std::min(1.0F, settings_.shape_blend + kBlendStep);
        return true;
    case LessonCommand::DecreaseRadius:
        settings_.sphere_radius = std::max(0.25F, settings_.sphere_radius - kRadiusStep);
        return true;
    case LessonCommand::IncreaseRadius:
        settings_.sphere_radius = std::min(1.20F, settings_.sphere_radius + kRadiusStep);
        return true;
    case LessonCommand::DecreaseRepeat:
        settings_.repeat = std::max(0.80F, settings_.repeat - kRepeatStep);
        return true;
    case LessonCommand::IncreaseRepeat:
        settings_.repeat = std::min(5.0F, settings_.repeat + kRepeatStep);
        return true;
    case LessonCommand::DecreaseLight:
        settings_.light_energy = std::max(0.0F, settings_.light_energy - kLightStep);
        return true;
    case LessonCommand::IncreaseLight:
        settings_.light_energy = std::min(2.5F, settings_.light_energy + kLightStep);
        return true;
    case LessonCommand::ReloadShaders:
        reload_requested_ = true;
        return true;
    case LessonCommand::CaptureFrame:
        capture_requested_ = true;
        return true;
    case LessonCommand::TogglePause:
        paused_ = !paused_;
        return true;
    case LessonCommand::Close:
        should_close_ = true;
        return true;
    }
    return false;
}

bool LessonSession::take_reload_request() noexcept {
    const bool requested = reload_requested_;
    reload_requested_ = false;
    return requested;
}

bool LessonSession::take_capture_request() noexcept {
    const bool requested = capture_requested_;
    capture_requested_ = false;
    return requested;
}

}  // namespace shader_sketchbook
