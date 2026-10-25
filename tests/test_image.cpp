#include "shader_sketchbook/image.hpp"

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

    Image left(2, 1);
    left.at(0, 0) = {0, 64, 128, 255};
    left.at(1, 0) = {255, 192, 32, 255};

    Image identical = left;
    require(mean_absolute_error(left, identical) == 0.0F, "identical images have zero error");

    Image changed = left;
    changed.at(0, 0).r = 255;
    const float expected_error = 255.0F / (2.0F * 3.0F * 255.0F);
    require(std::abs(mean_absolute_error(left, changed) - expected_error) < 0.000001F,
            "error is normalized across RGB channels");

    Image different_size(1, 1);
    require(std::isinf(mean_absolute_error(left, different_size)), "different dimensions produce infinite error");

    std::cout << "image_core: PASS\n";
    std::cout << "single_channel_error: " << mean_absolute_error(left, changed) << '\n';
    return EXIT_SUCCESS;
}
