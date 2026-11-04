// clang-format off
#include <glad/gl.h>
// clang-format on
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <print>
#include <string>

#include "bmp.hpp"
#include "raw_image.hpp"

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <image file name inside test_images/>\n";
        return 1;
    }
    const std::string image_path = std::string(IMAGES) + argv[1];

    int width, height;
    uint8_t* data = (uint8_t*) read_bmp(image_path, width, height);
    for(int i = 0; i < height; ++i) {
        for(int j = 0; j < width; ++j) {
            int R = data[(i * width + j) * 3 + 0];
            int G = data[(i * width + j) * 3 + 1];
            int B = data[(i * width + j) * 3 + 2];
            std::print("{:03} {:03} {:03}\t\t", R, G, B);
        }
        std::print("\n");
    }
    return 0;
}
