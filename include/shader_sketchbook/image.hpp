#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace shader_sketchbook {

struct Rgba8 {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    friend bool operator==(const Rgba8& left, const Rgba8& right) {
        return left.r == right.r && left.g == right.g && left.b == right.b && left.a == right.a;
    }
};

class Image {
public:
    Image() = default;
    Image(int width, int height);

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }

    Rgba8& at(int x, int y);
    const Rgba8& at(int x, int y) const;

    const std::vector<Rgba8>& pixels() const noexcept { return pixels_; }
    std::vector<Rgba8>& pixels() noexcept { return pixels_; }

    void save_ppm(const std::filesystem::path& path) const;
    static Image load_ppm(const std::filesystem::path& path);

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Rgba8> pixels_;
};

float mean_absolute_error(const Image& left, const Image& right);

}  // namespace shader_sketchbook


