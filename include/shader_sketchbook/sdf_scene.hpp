#pragma once

#include "shader_sketchbook/image.hpp"

#include <cstdint>

namespace shader_sketchbook {

struct Vec3 {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;

    constexpr Vec3() = default;
    constexpr Vec3(float x_value, float y_value, float z_value)
        : x(x_value), y(y_value), z(z_value) {}
};

struct SceneSettings {
    float shape_blend = 0.35F;
    float sphere_radius = 0.72F;
    float repeat = 2.70F;
    float light_energy = 1.15F;
};

float signed_distance(Vec3 point, const SceneSettings& settings) noexcept;
Image render_reference(int width, int height, const SceneSettings& settings);
std::uint64_t image_checksum(const Image& image) noexcept;

}  // namespace shader_sketchbook
