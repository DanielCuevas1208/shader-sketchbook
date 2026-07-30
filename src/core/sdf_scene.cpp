#include "shader_sketchbook/sdf_scene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace shader_sketchbook {
namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr float kEpsilon = 0.0015F;

Vec3 operator+(Vec3 left, Vec3 right) noexcept { return {left.x + right.x, left.y + right.y, left.z + right.z}; }
Vec3 operator-(Vec3 left, Vec3 right) noexcept { return {left.x - right.x, left.y - right.y, left.z - right.z}; }
Vec3 operator-(Vec3 value) noexcept { return {-value.x, -value.y, -value.z}; }
Vec3 operator*(Vec3 value, float scalar) noexcept { return {value.x * scalar, value.y * scalar, value.z * scalar}; }
Vec3 operator*(float scalar, Vec3 value) noexcept { return value * scalar; }
Vec3 operator/(Vec3 value, float scalar) noexcept { return value * (1.0F / scalar); }

float dot(Vec3 left, Vec3 right) noexcept { return left.x * right.x + left.y * right.y + left.z * right.z; }
Vec3 cross(Vec3 left, Vec3 right) noexcept {
    return {left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z, left.x * right.y - left.y * right.x};
}
float length(Vec3 value) noexcept { return std::sqrt(dot(value, value)); }
Vec3 normalize(Vec3 value) noexcept {
    const float magnitude = length(value);
    return magnitude > std::numeric_limits<float>::epsilon() ? value / magnitude : Vec3{0.0F, 1.0F, 0.0F};
}
Vec3 abs_vec(Vec3 value) noexcept { return {std::abs(value.x), std::abs(value.y), std::abs(value.z)}; }
Vec3 max_vec(Vec3 value, float minimum) noexcept { return {std::max(value.x, minimum), std::max(value.y, minimum), std::max(value.z, minimum)}; }
float max_component(Vec3 value) noexcept { return std::max(value.x, std::max(value.y, value.z)); }
float clamp01(float value) noexcept { return std::clamp(value, 0.0F, 1.0F); }

float smooth_min(float left, float right, float width) noexcept {
    const float safe_width = std::max(width, 0.0001F);
    const float blend = clamp01(0.5F + 0.5F * (right - left) / safe_width);
    return (right * (1.0F - blend) + left * blend) - safe_width * blend * (1.0F - blend);
}

float sd_box(Vec3 point, Vec3 half_size) noexcept {
    const Vec3 distance = abs_vec(point) - half_size;
    const Vec3 outside = max_vec(distance, 0.0F);
    return length(outside) + std::min(max_component(distance), 0.0F);
}

float centered_repeat(float coordinate, float cell) noexcept {
    const float wrapped = coordinate - cell * std::floor(coordinate / cell + 0.5F);
    return wrapped;
}

float object_distance(Vec3 point, const SceneSettings& settings) noexcept {
    const float radius = std::max(settings.sphere_radius, 0.1F);
    const float blend = std::clamp(settings.shape_blend, 0.0F, 1.0F);
    const float cell = std::max(settings.repeat, 0.6F);

    const float sphere = length(point) - radius;
    const float box = sd_box(point - Vec3{0.62F, 0.05F, 0.0F}, Vec3{0.48F, 0.48F, 0.48F});

    Vec3 repeated{centered_repeat(point.x, cell), point.y + 0.08F, centered_repeat(point.z, cell)};
    const float satellite = length(repeated) - radius * 0.34F;
    const float fused = smooth_min(sphere, box, 0.08F + blend * 0.32F);
    return smooth_min(fused, satellite, 0.04F + blend * 0.12F);
}

float distance_at(Vec3 point, const SceneSettings& settings) noexcept {
    const float ground = point.y + 1.02F;
    return std::min(object_distance(point, settings), ground);
}

Vec3 tone_map(Vec3 color) noexcept {
    color = {color.x / (1.0F + color.x), color.y / (1.0F + color.y), color.z / (1.0F + color.z)};
    constexpr float gamma = 1.0F / 2.2F;
    return {std::pow(std::max(color.x, 0.0F), gamma), std::pow(std::max(color.y, 0.0F), gamma), std::pow(std::max(color.z, 0.0F), gamma)};
}

std::uint8_t to_byte(float value) noexcept {
    return static_cast<std::uint8_t>(std::clamp(value * 255.0F + 0.5F, 0.0F, 255.0F));
}

Rgba8 shade(Vec3 ray_origin, Vec3 ray_direction, const SceneSettings& settings) noexcept {
    Vec3 background{0.015F + 0.035F * (ray_direction.y + 1.0F), 0.025F, 0.07F + 0.08F * (ray_direction.y + 1.0F)};
    float distance_travelled = 0.0F;
    bool hit = false;
    Vec3 point{};

    for (int step = 0; step < 96; ++step) {
        point = ray_origin + ray_direction * distance_travelled;
        const float distance = distance_at(point, settings);
        if (distance < kEpsilon) {
            hit = true;
            break;
        }
        distance_travelled += distance;
        if (distance_travelled > 12.0F) {
            break;
        }
    }

    if (!hit) {
        return {to_byte(background.x), to_byte(background.y), to_byte(background.z), 255};
    }

    const float normal_step = 0.0025F;
    const Vec3 normal = normalize(Vec3{
        distance_at(point + Vec3{normal_step, 0.0F, 0.0F}, settings) - distance_at(point - Vec3{normal_step, 0.0F, 0.0F}, settings),
        distance_at(point + Vec3{0.0F, normal_step, 0.0F}, settings) - distance_at(point - Vec3{0.0F, normal_step, 0.0F}, settings),
        distance_at(point + Vec3{0.0F, 0.0F, normal_step}, settings) - distance_at(point - Vec3{0.0F, 0.0F, normal_step}, settings)});
    const Vec3 light_direction = normalize(Vec3{-0.45F, 0.88F, 0.38F});
    const Vec3 view_direction = -ray_direction;
    const Vec3 half_direction = normalize(light_direction + view_direction);
    const float diffuse = std::max(dot(normal, light_direction), 0.0F);
    const float specular = std::pow(std::max(dot(normal, half_direction), 0.0F), 48.0F);
    const float ambient = 0.18F + 0.18F * clamp01(normal.y);
    const float occlusion = clamp01(1.0F - 7.0F * distance_at(point + normal * 0.08F, settings));

    const bool is_ground = point.y < -0.99F;
    Vec3 base_color{};
    if (is_ground) {
        const float tile = std::fmod(std::abs(std::floor(point.x * 1.6F) + std::floor(point.z * 1.6F)), 2.0F);
        base_color = tile < 1.0F ? Vec3{0.06F, 0.08F, 0.14F} : Vec3{0.09F, 0.12F, 0.20F};
    } else {
        const float hue = 0.5F + 0.5F * std::sin((point.x + point.z) * (2.3F + settings.shape_blend));
        base_color = {0.08F + 0.42F * hue, 0.50F - 0.20F * hue, 0.82F + 0.10F * hue};
    }

    const float energy = std::max(settings.light_energy, 0.0F);
    Vec3 color = base_color * (ambient + diffuse * energy) * occlusion;
    color = color + Vec3{0.18F, 0.23F, 0.38F} * (specular * energy);
    const float rim = std::pow(1.0F - std::max(dot(normal, view_direction), 0.0F), 3.0F);
    color = color + Vec3{0.12F, 0.25F, 0.55F} * rim * 0.32F;
    const Vec3 mapped = tone_map(color);
    return {to_byte(mapped.x), to_byte(mapped.y), to_byte(mapped.z), 255};
}

}  // namespace

float signed_distance(Vec3 point, const SceneSettings& settings) noexcept {
    return distance_at(point, settings);
}

Image render_reference(int width, int height, const SceneSettings& settings) {
    Image image(width, height);
    const Vec3 camera{3.45F, 1.85F, 4.35F};
    const Vec3 target{0.0F, -0.08F, 0.0F};
    const Vec3 forward = normalize(target - camera);
    const Vec3 right = normalize(cross(forward, Vec3{0.0F, 1.0F, 0.0F}));
    const Vec3 up = cross(right, forward);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float screen_x = (2.0F * (static_cast<float>(x) + 0.5F) / static_cast<float>(width) - 1.0F) * aspect;
            const float screen_y = 1.0F - 2.0F * (static_cast<float>(y) + 0.5F) / static_cast<float>(height);
            const Vec3 direction = normalize(forward + right * (screen_x * 0.88F) + up * (screen_y * 0.88F));
            image.at(x, y) = shade(camera, direction, settings);
        }
    }
    return image;
}

std::uint64_t image_checksum(const Image& image) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const Rgba8 pixel : image.pixels()) {
        hash ^= pixel.r;
        hash *= 1099511628211ULL;
        hash ^= pixel.g;
        hash *= 1099511628211ULL;
        hash ^= pixel.b;
        hash *= 1099511628211ULL;
    }
    return hash;
}

}  // namespace shader_sketchbook
