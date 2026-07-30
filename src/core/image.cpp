#include "shader_sketchbook/image.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

namespace shader_sketchbook {

Image::Image(int width, int height)
    : width_(width), height_(height), pixels_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Image dimensions must be positive");
    }
}

Rgba8& Image::at(int x, int y) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        throw std::out_of_range("Image coordinate is outside the image");
    }
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

const Rgba8& Image::at(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        throw std::out_of_range("Image coordinate is outside the image");
    }
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

void Image::save_ppm(const std::filesystem::path& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open PPM output: " + path.string());
    }
    file << "P6\n" << width_ << ' ' << height_ << "\n255\n";
    for (const Rgba8 pixel : pixels_) {
        file.put(static_cast<char>(pixel.r));
        file.put(static_cast<char>(pixel.g));
        file.put(static_cast<char>(pixel.b));
    }
    if (!file) {
        throw std::runtime_error("Could not write PPM output: " + path.string());
    }
}

Image Image::load_ppm(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open PPM input: " + path.string());
    }

    std::string magic;
    int width = 0;
    int height = 0;
    int max_value = 0;
    file >> magic >> width >> height >> max_value;
    if (!file || magic != "P6" || max_value != 255) {
        throw std::runtime_error("PPM must use binary P6 with an 8-bit channel");
    }
    file.get();

    Image image(width, height);
    for (Rgba8& pixel : image.pixels_) {
        char red = 0;
        char green = 0;
        char blue = 0;
        file.get(red).get(green).get(blue);
        if (!file) {
            throw std::runtime_error("PPM ended before all pixels were read");
        }
        pixel.r = static_cast<std::uint8_t>(static_cast<unsigned char>(red));
        pixel.g = static_cast<std::uint8_t>(static_cast<unsigned char>(green));
        pixel.b = static_cast<std::uint8_t>(static_cast<unsigned char>(blue));
        pixel.a = 255;
    }
    return image;
}

float mean_absolute_error(const Image& left, const Image& right) {
    if (left.width() != right.width() || left.height() != right.height()) {
        return std::numeric_limits<float>::infinity();
    }

    double total = 0.0;
    const std::vector<Rgba8>& left_pixels = left.pixels();
    const std::vector<Rgba8>& right_pixels = right.pixels();
    for (std::size_t index = 0; index < left_pixels.size(); ++index) {
        total += std::abs(static_cast<int>(left_pixels[index].r) - static_cast<int>(right_pixels[index].r));
        total += std::abs(static_cast<int>(left_pixels[index].g) - static_cast<int>(right_pixels[index].g));
        total += std::abs(static_cast<int>(left_pixels[index].b) - static_cast<int>(right_pixels[index].b));
    }
    const double denominator = static_cast<double>(left_pixels.size()) * 3.0 * 255.0;
    return static_cast<float>(total / denominator);
}

}  // namespace shader_sketchbook
