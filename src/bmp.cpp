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

bool BitmapFileHeader::validate_header() {
    return bfType == 0x4d42 &&
           bfSize == 14 & bfReserved1 == 0 & bfReserved2 == 0;
}

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

void BitmapHeader::read_header(RawImage& buffer) {
    biSize = buffer.u32le();
    switch(biSize) {
        case 40:
            version = Info;
            break;
        case 108:
            version = V4;
            break;
        case 124:
            version = V5;
            break;
        default:
            std::cerr << "Invalid bmp image\n";
            std::terminate();
            break;
    }


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
    if(version == Info &&
       (biCompression == BI_BITFIELDS || biCompression == BI_ALPHABITFIELDS)) {
        bRedMask = buffer.u32le();
        bGreenMask = buffer.u32le();
        bBlueMask = buffer.u32le();
        if(biCompression == BI_ALPHABITFIELDS) {
            bAlphaMask = buffer.u32le();
        }
    }

    if(version != Info) {
        bRedMask = buffer.u32le();
        bGreenMask = buffer.u32le();
        bBlueMask = buffer.u32le();
        bAlphaMask = buffer.u32le();
        bCSType = buffer.u32le();
        buffer.skip(36);
        bGammaRed = buffer.u32le();
        bGammaGreen = buffer.u32le();
        bGammaBlue = buffer.u32le();
    }
    if(version == V5) {
        bIntent = buffer.u32le();
        bProfileData = buffer.u32le();
        bProfileSize = buffer.u32le();
        bReserved = buffer.u32le();
    }
    if((biBitCount == 16 || biBitCount == 32) &&
       (biCompression != BI_ALPHABITFIELDS && biCompression != BI_BITFIELDS)) {
        if(biBitCount == 16) {
            bRedMask = 0x7C00;
            bGreenMask = 0x03E0;
            bBlueMask = 0x001F;
        }
        else {
            bRedMask = 0x00'FF'00'00;
            bGreenMask = 0x00'00'FF'00;
            bBlueMask = 0x00'00'00'FF;
        }
        bAlphaMask = 0;
    }
}

void* read_bmp(const std::string& path, int& width, int& height,
               int& channels) {
    channels = 3; // 4 if BI_ALPHABITFIELDS

    RawImage buffer(path);
    uint8_t* color_data = nullptr;
    BitmapFileHeader file_header;
    BitmapHeader bitmap_header;
    file_header.read_header(buffer);

    if(!file_header.validate_header()) {
        std::cerr << "Not valid bmp.\n";
        std::terminate();
    }
    file_header.read_header(buffer);
    if(bitmap_header.biBitCount != 24) {
        std::cerr << "Only support 24bit pixel data\n";
        std::terminate();
    }
    if(bitmap_header.biHeight < 1) {
        std::cerr << "Only support positive heights\n";
        std::terminate();
    }
    if(bitmap_header.biClrUsed != 0) {
        std::cerr << "Does not support color tables\n";
        std::terminate();
    }
    if(bitmap_header.biCompression != BI_RGB) {
        std::cerr << "Only supports uncompressed RGB images\n";
        std::terminate();
    }

    width = bitmap_header.biWidth;
    height = bitmap_header.biHeight;
    int stride = ((((width * bitmap_header.biBitCount) + 31) & ~31) >> 3);
    int padding_size = stride - width * 3;

    color_data = (uint8_t*) malloc((size_t) width * height * 3);
    if(color_data == nullptr) {
        std::cerr << "Memory allocation failed\n";
        std::terminate();
    }

    for(int i = height - 1; i >= 0; --i) {
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