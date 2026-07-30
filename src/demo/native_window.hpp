#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <functional>
#include <string>

namespace shader_sketchbook::demo {

class NativeWindow {
public:
    NativeWindow() = default;
    ~NativeWindow();

    bool create(int width, int height, const char* title);
    void pump_messages();
    void close() noexcept { should_close_ = true; }
    bool should_close() const noexcept { return should_close_; }
    HWND handle() const noexcept { return window_; }
    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    void set_title(const std::string& title);
    void set_key_handler(std::function<void(unsigned int)> handler) { key_handler_ = std::move(handler); }

private:
    static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
    bool register_class();

    HWND window_ = nullptr;
    int width_ = 1;
    int height_ = 1;
    bool should_close_ = false;
    std::function<void(unsigned int)> key_handler_;
};

}  // namespace shader_sketchbook::demo

