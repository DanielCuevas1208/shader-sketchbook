#include "shader_sketchbook/sdf_scene.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

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

    const SceneSettings settings{};
    require(std::abs(signed_distance({0.0F, 0.0F, 0.0F}, settings) + settings.sphere_radius) < 0.0001F,
            "the center distance matches the configured sphere radius");
    require(std::abs(signed_distance({0.0F, -1.02F, 0.0F}, settings)) < 0.0001F,
            "the ground plane crosses its configured height");

    const SceneSettings larger = [] {
        SceneSettings value{};
        value.sphere_radius = 1.0F;
        return value;
    }();
    require(signed_distance({0.8F, 0.0F, 0.0F}, larger) < signed_distance({0.8F, 0.0F, 0.0F}, settings),
            "radius changes the signed-distance field");

    const Image image = render_reference(32, 18, settings);
    const Image variant = render_reference(32, 18, SceneSettings{0.75F, 0.72F, 2.70F, 1.15F});
    require(image.width() == 32 && image.height() == 18, "reference render preserves dimensions");
    require(image_checksum(image) != 0, "reference render produces a non-empty checksum");
    require(mean_absolute_error(image, variant) > 0.001F, "blend control changes the reference image");

    std::cout << "sdf_core: PASS\n";
    std::cout << "default_checksum: " << image_checksum(image) << '\n';
    return EXIT_SUCCESS;
}
