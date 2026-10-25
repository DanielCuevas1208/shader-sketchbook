#pragma once

#include "shader_sketchbook/sdf_scene.hpp"

#include <cstddef>
#include <filesystem>
#include <string>

namespace shader_sketchbook {

enum class LessonCommand {
    PreviousLesson,
    NextLesson,
    DecreaseBlend,
    IncreaseBlend,
    DecreaseRadius,
    IncreaseRadius,
    DecreaseRepeat,
    IncreaseRepeat,
    DecreaseLight,
    IncreaseLight,
    ReloadShaders,
    CaptureFrame,
    TogglePause,
    Close,
};

class LessonSession {
public:
    LessonSession() = default;

    bool load_shader_assets(const std::filesystem::path& shader_directory);
    bool handle(LessonCommand command);

    const SceneSettings& settings() const noexcept { return settings_; }
    std::size_t lesson_index() const noexcept { return lesson_index_; }
    bool paused() const noexcept { return paused_; }
    bool should_close() const noexcept { return should_close_; }
    const std::string& status() const noexcept { return status_; }

    bool take_reload_request() noexcept;
    bool take_capture_request() noexcept;

private:
    SceneSettings settings_{};
    std::size_t lesson_index_ = 0;
    bool paused_ = false;
    bool should_close_ = false;
    bool reload_requested_ = false;
    bool capture_requested_ = false;
    std::string status_ = "starting";
};

}  // namespace shader_sketchbook
