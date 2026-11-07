#include "bmp.hpp"

#include <bit>
#include <cassert>
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
    return bfType == 0x4d42 && bfReserved1 == 0 && bfReserved2 == 0;
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
            bAlphaMask = buffer.u32le();  // BI_BITFIELDS-el is működjön, de
                                          // V2-t ellenőrizni kell
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

void ColourTable::read_table(RawImage& buffer, int colour_count) {
    if(data) {
        delete_data();
    }
    data = (RGBQuad*) malloc(sizeof(RGBQuad) * colour_count);
    size = colour_count;
    if(!data) {
        std::cerr << "Allocation error\n";
        std::terminate();
    }
    for(int i = 0; i < colour_count; ++i) {
        data[i].B = buffer.u8();
        data[i].G = buffer.u8();
        data[i].R = buffer.u8();
        buffer.skip(1);
    }
}

void ColourTable::delete_data() {
    free(data);
    data = nullptr;
}

ColourTable::~ColourTable() { delete_data(); }

RGBQuad ColourTable::operator[](size_t i) const {
    if(i < size) {
        return data[i];
    }
    else {
        std::cerr << "Index out of bounds\n";
        std::terminate();
    }
}

uint8_t normalise_to_u8(uint32_t value, uint32_t max_value) {
    return value / max_value * 255;
}

int bitfield_shift_value(uint32_t bitfield) {
    for(int i = 0; i < 32; ++i) {
        if(bitfield & (1 << i)) {
            return i;
        }
    }
    return -1;
}

void read_4bpp_pixel_data(int width, int height, RawImage& buffer,
                          uint8_t* colour_data, const ColourTable& colours,
                          int stride) {
    for(int i = height - 1; i >= 0; --i) {
        int j = 0;
        while(j < stride * 8) {
            uint8_t byte = buffer.u8();
            if(j < width * 4) {
                int index = (byte & 0xF0) >> 4;
                colour_data[(i * width + j / 4) * 3 + 0] = colours[index].R;
                colour_data[(i * width + j / 4) * 3 + 1] = colours[index].G;
                colour_data[(i * width + j / 4) * 3 + 2] = colours[index].B;
            }
            j += 4;
            if(j < width * 4) {
                int index = byte & 0xF;
                colour_data[(i * width + j / 4) * 3 + 0] = colours[index].R;
                colour_data[(i * width + j / 4) * 3 + 1] = colours[index].G;
                colour_data[(i * width + j / 4) * 3 + 2] = colours[index].B;
            }
            j += 4;
        }
    }
}

void read_1bpp_pixel_data(int width, int height, RawImage& buffer,
                          uint8_t* colour_data, const ColourTable& colours,
                          int stride) {
    for(int i = height - 1; i >= 0; --i) {
        int j = 0;
        while(j < stride * 8) {
            uint8_t byte = buffer.u8();
            for(int k = 0; k + j < width && k < 8; ++k) {
                int index = ((byte >> (7 - k)) & 0x1);
                colour_data[(i * width + j / 8 + k) * 3 + 0] = colours[index].R;
                colour_data[(i * width + j / 8 + k) * 3 + 1] = colours[index].G;
                colour_data[(i * width + j / 8 + k) * 3 + 2] = colours[index].B;
            }
            j += 8;
        }
    }
}

void read_8bpp_pixel_data(int width, int height, RawImage& buffer,
                          uint8_t* colour_data, const ColourTable& colours,
                          int stride) {
    int padding_size = stride - width;
    for(int i = height - 1; i >= 0; --i) {
        for(int j = 0; j < width; ++j) {
            uint8_t index = buffer.u8();
            colour_data[(i * width + j) * 3 + 0] = colours[index].R;
            colour_data[(i * width + j) * 3 + 1] = colours[index].G;
            colour_data[(i * width + j) * 3 + 2] = colours[index].B;
        }
        buffer.skip(padding_size);
    }
}

void read_24bpp_pixel_data(int width, int height, RawImage& buffer,
                           uint8_t* colour_data, int stride) {
    int padding_size = stride - width * 3;
    for(int i = height - 1; i >= 0; --i) {
        for(int j = 0; j < width; ++j) {
            uint8_t R, G, B;
            B = buffer.u8();
            G = buffer.u8();
            R = buffer.u8();
            colour_data[(i * width + j) * 3 + 0] = R;
            colour_data[(i * width + j) * 3 + 1] = G;
            colour_data[(i * width + j) * 3 + 2] = B;
        }
        buffer.skip(padding_size);
    }
}

void read_16bpp_pixel_data(int width, int height, RawImage& buffer, uint8_t* colour_data,
               int stride, uint32_t red_mask, uint32_t green_mask,
               uint32_t blue_mask) {
    int padding_size = stride - width * 2;
    uint32_t red_max = (1 << std::popcount(red_mask)) - 1;
    uint32_t green_max = (1 << std::popcount(green_mask)) - 1;
    uint32_t blue_max = (1 << std::popcount(blue_mask)) - 1;
    int red_field_shift = bitfield_shift_value(red_mask);
    int green_field_shift = bitfield_shift_value(green_mask);
    int blue_field_shift = bitfield_shift_value(blue_mask);
    for(int i = height - 1; i >= 0; --i) {
        for(int j = 0; j < width; ++j) {
            uint32_t pixel = buffer.u16le();
            uint32_t red_value = (pixel & red_mask) >> red_field_shift;
            uint32_t green_value = (pixel & green_mask) >> green_field_shift;
            uint32_t blue_value = (pixel & blue_mask) >> blue_field_shift;
            colour_data[(i * width + j) * 3 + 0] =
                normalise_to_u8(red_value, red_max);
            colour_data[(i * width + j) * 3 + 1] =
                normalise_to_u8(green_value, green_max);
            colour_data[(i * width + j) * 3 + 2] =
                normalise_to_u8(blue_value, blue_max);
        }
        buffer.skip(padding_size);
    }
}

void* read_bmp(const std::string& path, int& width, int& height,
               int& channels) {
    channels = 3;  // 4 if BI_ALPHABITFIELDS

    RawImage buffer(path);
    uint8_t* colour_data = nullptr;
    BitmapFileHeader file_header;
    BitmapHeader bitmap_header;
    file_header.read_header(buffer);
    if(!file_header.validate_header()) {
        std::cerr << "Not valid bmp.\n";
        std::terminate();
    }


    bitmap_header.read_header(buffer);
    if(bitmap_header.biHeight < 1) {
        std::cerr << "Only support positive heights\n";
        std::terminate();
    }


    ColourTable colours;
    colours.read_table(buffer, bitmap_header.biClrUsed);

    assert(buffer.buffer == buffer.data + file_header.bfOffBits &&
           "Reading pixel data from wrong place\n");



    width = bitmap_header.biWidth;
    height = bitmap_header.biHeight;
    int stride = ((((width * bitmap_header.biBitCount) + 31) & ~31) >> 3);

    uint32_t red_mask = bitmap_header.bRedMask;
    uint32_t green_mask = bitmap_header.bGreenMask;
    uint32_t blue_mask = bitmap_header.bBlueMask;
    uint32_t alpha_mask = bitmap_header.bAlphaMask;

    colour_data = (uint8_t*) malloc((size_t) width * height * 3);
    if(colour_data == nullptr) {
        std::cerr << "Memory allocation failed\n";
        std::terminate();
    }

    switch(bitmap_header.biBitCount) {
        case 1:
            read_1bpp_pixel_data(width, height, buffer, colour_data, colours,
                                 stride);
            break;
        case 4:
            read_4bpp_pixel_data(width, height, buffer, colour_data, colours,
                                 stride);
            break;
        case 8:
            read_8bpp_pixel_data(width, height, buffer, colour_data, colours,
                                 stride);
            break;
        case 16: {
            read_16bpp_pixel_data(width, height, buffer, colour_data, stride, red_mask,
                      green_mask, blue_mask);
        } break;
        case 24:
            read_24bpp_pixel_data(width, height, buffer, colour_data, stride);
            break;
        case 32:
            // unimplemented
            break;
        default:
            std::cerr << "Invalid bitCount\n";
            std::terminate();
            break;
    }

    return colour_data;
}