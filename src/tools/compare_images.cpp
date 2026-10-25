#include "shader_sketchbook/image.hpp"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr float kDefaultThreshold = 0.002F;

void print_usage() {
    std::cerr << "Usage: sketchbook_image_compare <actual.ppm> <expected.ppm> [max-error]\n";
}

bool parse_threshold(const char* value, float& threshold) {
    try {
        std::size_t consumed = 0;
        const double parsed = std::stod(value, &consumed);
        if (consumed != std::string(value).size() || !std::isfinite(parsed) || parsed < 0.0 || parsed > 1.0) {
            return false;
        }
        threshold = static_cast<float>(parsed);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        print_usage();
        return EXIT_FAILURE;
    }

    float threshold = kDefaultThreshold;
    if (argc == 4 && !parse_threshold(argv[3], threshold)) {
        std::cerr << "FAIL: max-error must be a number from 0 to 1\n";
        return EXIT_FAILURE;
    }

    try {
        const shader_sketchbook::Image actual = shader_sketchbook::Image::load_ppm(argv[1]);
        const shader_sketchbook::Image expected = shader_sketchbook::Image::load_ppm(argv[2]);
        const float error = shader_sketchbook::mean_absolute_error(actual, expected);

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "mean_absolute_error: " << error << '\n';
        std::cout << "threshold: " << threshold << '\n';
        if (!std::isfinite(error)) {
            std::cerr << "FAIL: images have different dimensions\n";
            return EXIT_FAILURE;
        }
        if (error > threshold) {
            std::cerr << "FAIL: image difference exceeds the threshold\n";
            return EXIT_FAILURE;
        }
        std::cout << "image_compare: PASS\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
