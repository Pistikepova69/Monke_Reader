// clang-format off
#include <glad/gl.h>
// clang-format on
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
 
#include "display.hpp"
#include "shader.hpp"
#include "texture.hpp"
 
// Window size limits (in screen pixels) for the longer side of the image
constexpr int MIN_SIDE = 400;
constexpr int MAX_SIDE = 1000;
 
int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <image file name inside test_images/>\n";
        return 1;
    }
    const std::string image_path = std::string(IMAGES) + argv[1];
 
 
    // Window must be declared first: it is destroyed last, so every
    // other GL object is freed while the context is still alive.
    Window window(400, 400, "Image Reader");
    Shader shader(SHADER "vertex.vert", SHADER "fragment.frag");
    texture2D texture(image_path);
    Quad quad;
 
    // Tiny images: integer upscale (crisp pixels). Huge images: shrink to fit.
    int w = texture.get_width(), h = texture.get_height();
    int longest = std::max(w, h);
    if(longest < MIN_SIDE) {
        int scale = MIN_SIDE / longest;
        w *= scale;
        h *= scale;
    }
    else if(longest > MAX_SIDE) {
        double f = static_cast<double>(MAX_SIDE) / longest;
        w = std::max(1, static_cast<int>(std::lround(w * f)));
        h = std::max(1, static_cast<int>(std::lround(h * f)));
    }
    window.set_content_size(w, h);
    window.show();
 
    shader.use();
    shader.set_int("tex", 0);
 
    while(!window.should_close()) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
 
        texture.activate(GL_TEXTURE0);
        quad.draw();
 
        window.swap_and_poll();
    }
    return 0;
}
