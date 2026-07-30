#include "shader_sketchbook/sdf_scene.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#ifndef SKETCHBOOK_FIXTURE_DIR
#error "SKETCHBOOK_FIXTURE_DIR must point to the committed reference fixtures"
#endif

int main(int argc, char** argv) {
    using namespace shader_sketchbook;
    const SceneSettings settings{};
    const Image rendered = render_reference(32, 18, settings);
    const std::filesystem::path fixture = std::filesystem::path(SKETCHBOOK_FIXTURE_DIR) / "sdf_scene_reference.ppm";

    if (argc == 3 && std::string(argv[1]) == "--write-reference") {
        rendered.save_ppm(argv[2]);
        std::cout << "wrote " << argv[2] << '\n';
        return EXIT_SUCCESS;
    }

    const Image expected = Image::load_ppm(fixture);
    constexpr float max_cross_platform_error = 0.002F;
    const float error = mean_absolute_error(rendered, expected);
    if (error > max_cross_platform_error) {
        std::cerr << "FAIL: reference image changed; mean absolute error=" << error << '\n';
        std::cerr << "rendered checksum=" << image_checksum(rendered) << '\n';
        return EXIT_FAILURE;
    }

    const Image changed = render_reference(32, 18, SceneSettings{0.35F, 0.88F, 2.70F, 1.15F});
    if (mean_absolute_error(rendered, changed) <= 0.001F) {
        std::cerr << "FAIL: reference test did not detect a parameter change\n";
        return EXIT_FAILURE;
    }

    std::cout << "reference_image: PASS\n";
    std::cout << "fixture: " << fixture.string() << '\n';
    std::cout << "checksum: " << image_checksum(rendered) << '\n';
    return EXIT_SUCCESS;
}

