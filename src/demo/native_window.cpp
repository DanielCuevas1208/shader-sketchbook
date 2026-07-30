#include "native_window.hpp"

#include <utility>

namespace shader_sketchbook::demo {
namespace {

constexpr const char* kWindowClass = "ShaderSketchbookWindow";

}  // namespace

NativeWindow::~NativeWindow() {
    if (window_ != nullptr) {
        DestroyWindow(window_);
        window_ = nullptr;
    }
}

bool NativeWindow::register_class() {
    static bool registered = false;
    if (registered) {
        return true;
    }

    WNDCLASSA window_class{};
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = &NativeWindow::window_proc;
    window_class.hInstance = GetModuleHandleA(nullptr);
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    window_class.lpszClassName = kWindowClass;
    if (RegisterClassA(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }
    registered = true;
    return true;
}

bool NativeWindow::create(int width, int height, const char* title) {
    if (!register_class()) {
        return false;
    }
    width_ = width;
    height_ = height;
    RECT rectangle{0, 0, width, height};
    AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE);
    window_ = CreateWindowExA(
        0,
        kWindowClass,
        title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rectangle.right - rectangle.left,
        rectangle.bottom - rectangle.top,
        nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        this);
    if (window_ == nullptr) {
        return false;
    }
    ShowWindow(window_, SW_SHOW);
    UpdateWindow(window_);
    return true;
}

void NativeWindow::pump_messages() {
    MSG message{};
    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        if (message.message == WM_QUIT) {
            should_close_ = true;
        }
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

void NativeWindow::set_title(const std::string& title) {
    if (window_ != nullptr) {
        SetWindowTextA(window_, title.c_str());
    }
}

LRESULT CALLBACK NativeWindow::window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    NativeWindow* instance = reinterpret_cast<NativeWindow*>(GetWindowLongPtrA(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<const CREATESTRUCTA*>(l_param);
        instance = static_cast<NativeWindow*>(create->lpCreateParams);
        SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));
    }
    if (instance == nullptr) {
        return DefWindowProcA(window, message, w_param, l_param);
    }

    switch (message) {
    case WM_SIZE:
        instance->width_ = LOWORD(l_param);
        instance->height_ = HIWORD(l_param);
        break;
    case WM_KEYDOWN:
        if ((l_param & (1LL << 30)) == 0 && instance->key_handler_) {
            instance->key_handler_(static_cast<unsigned int>(w_param));
        }
        break;
    case WM_CLOSE:
        instance->should_close_ = true;
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        instance->should_close_ = true;
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcA(window, message, w_param, l_param);
}

}  // namespace shader_sketchbook::demo
