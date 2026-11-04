#include "bmp.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>

#include "raw_image.hpp"

void BitmapFileHeader::read_header(RawImage& buffer) {
    bfType = buffer.u16le();
    bfSize = buffer.u32le();
    bfReserved1 = buffer.u16le();
    bfReserved2 = buffer.u16le();
    bfOffBits = buffer.u32le();
}

bool BitmapFileHeader::validate_header() { return bfType == 0x4d42; }

void BitmapInfoHeader::read_header(RawImage& buffer) {
    biSize = buffer.u32le();
    biWidth = buffer.i32le();
    biHeight = buffer.i32le();
    biPlanes = buffer.u16le();
    biBitCount = buffer.u16le();
    biCompression = buffer.u32le();
    biSizeImage = buffer.u32le();
    biXPelsPerMeter = buffer.i32le();
    biYPelsPerMeter = buffer.i32le();
    biClrUsed = buffer.u32le();
    biClrImportant = buffer.u32le();
}

void* read_bmp(const std::string& path, int& width, int& height) {
    RawImage buffer(path);
    uint8_t* color_data = nullptr;
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    file_header.read_header(buffer);

    if(!file_header.validate_header()) {
        std::cerr << "Not valid bmp.\n";
        std::terminate();
    }
    info_header.read_header(buffer);
    if(info_header.biBitCount != 24) {
        std::cerr << "Only support 24bit pixel data\n";
        std::terminate();
    }
    if(info_header.biHeight < 1) {
        std::cerr << "Only support positive heights\n";
        std::terminate();
    }
    if(info_header.biClrUsed != 0) {
        std::cerr << "Does not support color tables\n";
        std::terminate();
    }
    if(info_header.biCompression != BI_RGB) {
        std::cerr << "Only supports uncompressed RGB images\n";
        std::terminate();
    }

    width = info_header.biWidth;
    height = info_header.biHeight;
    int stride = ((((width * info_header.biBitCount) + 31) & ~31) >> 3);
    int padding_size = stride - width * 3;

    color_data = (uint8_t*) malloc((size_t) width * height * 3);
    if(color_data == nullptr) {
        std::cerr << "Memory allocation failed\n";
        std::terminate();
    }

    for(int i = 0; i < height; ++i) {
        for(int j = 0; j < width; ++j) {
            uint8_t R, G, B;
            B = buffer.u8();
            G = buffer.u8();
            R = buffer.u8();
            color_data[(i * width + j) * 3 + 0] = R;
            color_data[(i * width + j) * 3 + 1] = G;
            color_data[(i * width + j) * 3 + 2] = B;
        }
        buffer.skip(padding_size);
    }
    return color_data;
}