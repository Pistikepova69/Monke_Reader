#include <iostream>
#include "bmp.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <utility>

#include "texture.hpp"

texture2D::texture2D(const std::string& path) { load(path); }

texture2D::texture2D(texture2D&& other) noexcept
    : id(std::exchange(other.id, 0)), width(other.width), height(other.height) {
}

texture2D& texture2D::operator=(texture2D&& other) noexcept {
    if(this != &other) {
        unload();
        id = std::exchange(other.id, 0);
        width = other.width;
        height = other.height;
    }
    return *this;
}

texture2D::~texture2D() { unload(); }

void texture2D::unload() {
    if(id) {
        glDeleteTextures(1, &id);
    }
    id = 0;
}

void texture2D::load(const std::string& path) {
    unload();

    int channels = 0;
    unsigned char* image = (unsigned char *) read_bmp(path, width, height, channels);
        //stbi_load(path.c_str(), &width, &height, &channels, 0);
    if(!image) {
        std::cerr << "Cannot open image: " + path + "\n";
        std::terminate();
    }

    GLenum fmt = GL_RGBA;
    GLenum ifmt = GL_RGBA8;
    // Make grayscale images show up as gray instead of red
    GLint swizzle[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};

    switch(channels) {
        case 1:
            fmt = GL_RED;
            ifmt = GL_R8;
            swizzle[1] = swizzle[2] = GL_RED;
            swizzle[3] = GL_ONE;
            break;
        case 2:
            fmt = GL_RG;
            ifmt = GL_RG8;
            swizzle[1] = swizzle[2] = GL_RED;
            swizzle[3] = GL_GREEN;
            break;
        case 3:
            fmt = GL_RGB;
            ifmt = GL_RGB8;
            break;
        default:
            break;  // 4 channels: RGBA
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) ifmt, width, height, 0, fmt,
                 GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void texture2D::use() const { glBindTexture(GL_TEXTURE_2D, id); }

void texture2D::activate(GLenum unit) const {
    glActiveTexture(unit);
    use();
}
