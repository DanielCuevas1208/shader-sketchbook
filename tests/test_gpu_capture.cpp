#include "native_window.hpp"
#include "opengl_renderer.hpp"

#include "shader_sketchbook/image.hpp"
#include "shader_sketchbook/sdf_scene.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#ifndef SKETCHBOOK_SHADER_DIR
#error "SKETCHBOOK_SHADER_DIR must point to the GLSL asset directory"
#endif

#ifndef SKETCHBOOK_GPU_FIXTURE
#error "SKETCHBOOK_GPU_FIXTURE must point to the GPU reference image"
#endif

namespace {

constexpr int kSkip = 77;
constexpr int kCaptureWidth = 64;
constexpr int kCaptureHeight = 36;
constexpr float kGpuTolerance = 0.08F;

void print_skip(const std::string& reason) {
    std::cout << "gpu_capture: SKIP\n";
    std::cout << "reason: " << reason << '\n';
}

bool context_unavailable(const std::string& status) {
    return status == "GetDC failed" ||
           status == "pixel format setup failed" ||
           status == "OpenGL context setup failed" ||
           status == "OpenGL shader functions are unavailable";
}

int capture(const std::filesystem::path& output) {
    using namespace shader_sketchbook;
    using namespace shader_sketchbook::demo;

    NativeWindow window;
    if (!window.create(kCaptureWidth, kCaptureHeight, "Shader Sketchbook GPU Smoke Test")) {
        print_skip("Win32 window could not be created");
        return kSkip;
    }
    ShowWindow(window.handle(), SW_HIDE);

    OpenGLRenderer renderer;
    if (!renderer.initialize(window.handle(), std::filesystem::path(SKETCHBOOK_SHADER_DIR))) {
        if (context_unavailable(renderer.status())) {
            print_skip(renderer.status());
            return kSkip;
        }
        std::cerr << "FAIL: OpenGL initialization failed: " << renderer.status() << '\n';
        return EXIT_FAILURE;
    }

    renderer.resize(kCaptureWidth, kCaptureHeight);
    renderer.render(SceneSettings{}, 0.0F);
    if (!renderer.capture_frame(output)) {
        std::cerr << "FAIL: framebuffer capture failed: " << renderer.status() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 1 && argc != 3) {
        std::cerr << "Usage: sketchbook_gpu_tests [--write-reference output.ppm]\n";
        return EXIT_FAILURE;
    }

    const std::filesystem::path output = argc == 3
                                             ? std::filesystem::path(argv[2])
                                             : std::filesystem::current_path() / "gpu_capture.ppm";
    const int capture_status = capture(output);
    if (capture_status != EXIT_SUCCESS) {
        return capture_status;
    }

    if (argc == 3) {
        std::cout << "gpu_capture: wrote " << output.string() << '\n';
        return EXIT_SUCCESS;
    }

    try {
        const shader_sketchbook::Image actual = shader_sketchbook::Image::load_ppm(output);
        const shader_sketchbook::Image expected =
            shader_sketchbook::Image::load_ppm(std::filesystem::path(SKETCHBOOK_GPU_FIXTURE));
        const float error = shader_sketchbook::mean_absolute_error(actual, expected);
        std::cout << "dimensions: " << actual.width() << "x" << actual.height() << '\n';
        std::cout << "mean_absolute_error: " << error << '\n';
        std::cout << "tolerance: " << kGpuTolerance << '\n';
        if (actual.width() != kCaptureWidth || actual.height() != kCaptureHeight) {
            std::cerr << "FAIL: GPU capture has unexpected dimensions" << '\n';
            return EXIT_FAILURE;
        }
        if (error > kGpuTolerance) {
            std::cerr << "FAIL: GPU framebuffer differs from the reference beyond the tolerance\n";
            return EXIT_FAILURE;
        }
        std::cout << "gpu_capture: PASS\n";
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
