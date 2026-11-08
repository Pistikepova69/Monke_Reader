#include "bmp.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>

#include "raw_image.hpp"

#define BI_RGB 0L
#define BI_RLE8 1L
#define BI_RLE4 2L
#define BI_BITFIELDS 3L
#define BI_JPEG 4L
#define BI_PNG 5L
#define BI_ALPHABITFIELDS 6L

struct BitmapFileHeader {
    uint16_t bfType;       // 'BM'!
    uint32_t bfSize;       // 14!
    uint16_t bfReserved1;  // 0!
    uint16_t bfReserved2;  // 0!
    uint32_t bfOffBits;    // mostly irrelevant

    void read_header(RawImage& buffer);
    bool validate_header();
};

struct BitmapHeader {
    enum valid_versions : uint8_t {
        Info = 1,
        V2 = 2,
        V3 = 3,
        V4 = 4,
        V5 = 5,
        None = 0
    };

    valid_versions version = None;


    uint32_t biSize = 0;          // 40 or 108 or 124!
    int32_t biWidth = 0;          // dimesnion of image
    int32_t biHeight = 0;         // dimesnion of image
    uint16_t biPlanes = 0;        // 1!
    uint16_t biBitCount = 0;      // bits per pixel (1, 4, 8, 16, 24, 32)
    uint32_t biCompression = 0;   // currently only BI_RGB maybe more?
    uint32_t biSizeImage = 0;     // irrelevant
    int32_t biXPelsPerMeter = 0;  // irrelevant
    int32_t biYPelsPerMeter = 0;  // irrelevant
    uint32_t biClrUsed = 0;       // colour table size
    uint32_t biClrImportant = 0;  // irrelevant
    // info header until this point
    uint32_t bRedMask = 0;
    // bitmasks to read color data, only if bpp = (16,  32)
    uint32_t bGreenMask = 0;
    // bitmasks to read color data, onli if bpp = (16, 32)
    uint32_t bBlueMask = 0;
    // bitmasks to read color data, onli if bpp = (16, 32)
    uint32_t bAlphaMask = 0;
    // bitmasks to read color data, onli if bpp = (16, 32)
    uint32_t bCSType = 0;      //  0x73524742 or 0x57696E20
    uint8_t bEndpoints[36];    // not the real field type, skip, not relevant
    uint32_t bGammaRed = 0;    // can be ignored
    uint32_t bGammaGreen = 0;  // can be ignored
    uint32_t bGammaBlue = 0;   // can be ignored
    // V4 until this point
    uint32_t bIntent = 0;       // irrelevant
    uint32_t bProfileData = 0;  // irrelevant
    uint32_t bProfileSize = 0;  // irrelevant
    uint32_t bReserved = 0;     // irrelevant (0!)
    void read_header(RawImage& buffer);
};

struct RGBColour {
    uint8_t R, G, B;
};

struct ColourTable {
    RGBColour* data = nullptr;
    size_t size = 0;
    void read_table(RawImage& buffer, int colour_count);
    RGBColour operator[](size_t i) const;
    void delete_data();
    ~ColourTable();
};

void BitmapFileHeader::read_header(RawImage& input_buffer) {
    bfType = input_buffer.u16le();
    bfSize = input_buffer.u32le();
    bfReserved1 = input_buffer.u16le();
    bfReserved2 = input_buffer.u16le();
    bfOffBits = input_buffer.u32le();
}

bool BitmapFileHeader::validate_header() {
    return bfType == 0x4d42 && bfReserved1 == 0 && bfReserved2 == 0;
}

void BitmapHeader::read_header(RawImage& input_buffer) {
    biSize = input_buffer.u32le();
    switch(biSize) {
        case 40:
            version = Info;
            break;
        case 52:
            version = V2;
            break;
        case 56:
            version = V3;
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


    biWidth = input_buffer.i32le();
    biHeight = input_buffer.i32le();
    biPlanes = input_buffer.u16le();
    biBitCount = input_buffer.u16le();
    biCompression = input_buffer.u32le();
    biSizeImage = input_buffer.u32le();
    biXPelsPerMeter = input_buffer.i32le();
    biYPelsPerMeter = input_buffer.i32le();
    biClrUsed = input_buffer.u32le();
    biClrImportant = input_buffer.u32le();
    if(version == Info &&
       (biCompression == BI_BITFIELDS || biCompression == BI_ALPHABITFIELDS)) {
        bRedMask = input_buffer.u32le();
        bGreenMask = input_buffer.u32le();
        bBlueMask = input_buffer.u32le();
        if(biCompression == BI_ALPHABITFIELDS) {
            bAlphaMask = input_buffer.u32le();
        }
        else {
            bAlphaMask = 0;
        }
    }
    if(version == V2 || version == V3) {
        bRedMask = input_buffer.u32le();
        bGreenMask = input_buffer.u32le();
        bBlueMask = input_buffer.u32le();
        if(version == V3) {
            bAlphaMask = input_buffer.u32le();
        }
    }

    else if(version != Info) {
        bRedMask = input_buffer.u32le();
        bGreenMask = input_buffer.u32le();
        bBlueMask = input_buffer.u32le();
        bAlphaMask = input_buffer.u32le();
        bCSType = input_buffer.u32le();
        input_buffer.skip(36);
        bGammaRed = input_buffer.u32le();
        bGammaGreen = input_buffer.u32le();
        bGammaBlue = input_buffer.u32le();
    }
    if(version == V5) {
        bIntent = input_buffer.u32le();
        bProfileData = input_buffer.u32le();
        bProfileSize = input_buffer.u32le();
        bReserved = input_buffer.u32le();
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
    if(biBitCount != 16 && biBitCount != 32) bAlphaMask = 0;
    if(biBitCount < 16 && biClrUsed == 0) {
        biClrUsed = 1 << biBitCount;
    }
}

void ColourTable::read_table(RawImage& input_buffer, int colour_count) {
    if(colour_count == 0) {
        return;
    }
    if(data) {
        delete_data();
    }
    data = (RGBColour*) malloc(sizeof(RGBColour) * colour_count);
    size = colour_count;
    if(!data) {
        std::cerr << "Allocation error\n";
        std::terminate();
    }
    for(int i = 0; i < colour_count; ++i) {
        data[i].B = input_buffer.u8();
        data[i].G = input_buffer.u8();
        data[i].R = input_buffer.u8();
        input_buffer.skip(1);
    }
}

void ColourTable::delete_data() {
    free(data);
    data = nullptr;
}

ColourTable::~ColourTable() { delete_data(); }

RGBColour ColourTable::operator[](size_t i) const {
    if(i < size) {
        return data[i];
    }
    else {
        std::cerr << "Index out of bounds\n";
        std::terminate();
    }
}

static uint8_t normalise_to_u8(uint32_t value, uint32_t max_value) {
    return (255 * value) / max_value;
}

static int bitfield_shift_value(uint32_t bitfield) {
    for(int i = 0; i < 32; ++i) {
        if(bitfield & (1 << i)) {
            return i;
        }
    }
    return 65;  // to zero out shifted value
}

static void read_4bpp_pixel_data(int width, int height, RawImage& input_buffer,
                                 uint8_t* output_buffer,
                                 const ColourTable& colours, int stride) {
    for(int i = height - 1; i >= 0; --i) {
        int j = 0;
        while(j < stride * 8) {
            uint8_t byte = input_buffer.u8();
            if(j < width * 4) {
                int index = (byte & 0xF0) >> 4;
                output_buffer[(i * width + j / 4) * 3 + 0] = colours[index].R;
                output_buffer[(i * width + j / 4) * 3 + 1] = colours[index].G;
                output_buffer[(i * width + j / 4) * 3 + 2] = colours[index].B;
            }
            j += 4;
            if(j < width * 4) {
                int index = byte & 0xF;
                output_buffer[(i * width + j / 4) * 3 + 0] = colours[index].R;
                output_buffer[(i * width + j / 4) * 3 + 1] = colours[index].G;
                output_buffer[(i * width + j / 4) * 3 + 2] = colours[index].B;
            }
            j += 4;
        }
    }
}

static void read_1bpp_pixel_data(int width, int height, RawImage& input_buffer,
                                 uint8_t* output_buffer,
                                 const ColourTable& colours, int stride) {
    for(int i = height - 1; i >= 0; --i) {
        int j = 0;
        while(j < stride * 8) {
            uint8_t byte = input_buffer.u8();
            for(int k = 0; k + j < width && k < 8; ++k) {
                int index = ((byte >> (7 - k)) & 0x1);
                output_buffer[(i * width + j + k) * 3 + 0] = colours[index].R;
                output_buffer[(i * width + j + k) * 3 + 1] = colours[index].G;
                output_buffer[(i * width + j + k) * 3 + 2] = colours[index].B;
            }
            j += 8;
        }
    }
}

static void read_8bpp_pixel_data(int width, int height, RawImage& input_buffer,
                                 uint8_t* output_buffer,
                                 const ColourTable& colours, int stride) {
    int padding_size = stride - width;
    for(int i = height - 1; i >= 0; --i) {
        for(int j = 0; j < width; ++j) {
            uint8_t index = input_buffer.u8();
            output_buffer[(i * width + j) * 3 + 0] = colours[index].R;
            output_buffer[(i * width + j) * 3 + 1] = colours[index].G;
            output_buffer[(i * width + j) * 3 + 2] = colours[index].B;
        }
        input_buffer.skip(padding_size);
    }
}

static void read_24bpp_pixel_data(int width, int height, RawImage& input_buffer,
                                  uint8_t* output_buffer, int stride) {
    int padding_size = stride - width * 3;
    for(int i = height - 1; i >= 0; --i) {
        for(int j = 0; j < width; ++j) {
            uint8_t R, G, B;
            B = input_buffer.u8();
            G = input_buffer.u8();
            R = input_buffer.u8();
            output_buffer[(i * width + j) * 3 + 0] = R;
            output_buffer[(i * width + j) * 3 + 1] = G;
            output_buffer[(i * width + j) * 3 + 2] = B;
        }
        input_buffer.skip(padding_size);
    }
}

static void read_bitfield_data(int width, int height, RawImage& input_buffer,
                               uint8_t* output_buffer, int stride,
                               uint32_t red_mask, uint32_t green_mask,
                               uint32_t blue_mask, uint32_t alpha_mask,
                               int channels, int bpp) {
    if(bpp != 32 && bpp != 16) {
        std::cerr << "Only 16 and 32 bpp are valid bitmap sizes\n";
        std::terminate();
    }
    int padding_size = stride - width * (bpp == 16 ? 2 : 4);
    uint32_t red_max = (1 << std::popcount(red_mask)) - 1;
    uint32_t green_max = (1 << std::popcount(green_mask)) - 1;
    uint32_t blue_max = (1 << std::popcount(blue_mask)) - 1;
    uint32_t alpha_max = (1 << std::popcount(alpha_mask)) - 1;
    int red_field_shift = bitfield_shift_value(red_mask);
    int green_field_shift = bitfield_shift_value(green_mask);
    int blue_field_shift = bitfield_shift_value(blue_mask);
    int alpha_field_shift = bitfield_shift_value(alpha_mask);


    for(int i = height - 1; i >= 0; --i) {
        for(int j = 0; j < width; ++j) {
            uint32_t pixel =
                bpp == 16 ? input_buffer.u16le() : input_buffer.u32le();
            uint32_t red_value = (pixel & red_mask) >> red_field_shift;
            uint32_t green_value = (pixel & green_mask) >> green_field_shift;
            uint32_t blue_value = (pixel & blue_mask) >> blue_field_shift;
            uint32_t alpha_value = (pixel & alpha_mask) >> alpha_field_shift;

            output_buffer[(i * width + j) * channels + 0] =
                normalise_to_u8(red_value, red_max);
            output_buffer[(i * width + j) * channels + 1] =
                normalise_to_u8(green_value, green_max);
            output_buffer[(i * width + j) * channels + 2] =
                normalise_to_u8(blue_value, blue_max);
            if(alpha_mask) {
                output_buffer[(i * width + j) * channels + 3] =
                    normalise_to_u8(alpha_value, alpha_max);
            }
        }
        input_buffer.skip(padding_size);
    }
}

static void read_16bpp_pixel_data(int width, int height, RawImage& input_buffer,
                                  uint8_t* output_buffer, int stride,
                                  uint32_t red_mask, uint32_t green_mask,
                                  uint32_t blue_mask, uint32_t alpha_mask,
                                  int channels) {
    read_bitfield_data(width, height, input_buffer, output_buffer, stride,
                       red_mask, green_mask, blue_mask, alpha_mask, channels,
                       16);
}

static void read_32bpp_pixel_data(int width, int height, RawImage& input_buffer,
                                  uint8_t* output_buffer, int stride,
                                  uint32_t red_mask, uint32_t green_mask,
                                  uint32_t blue_mask, uint32_t alpha_mask,
                                  int channels) {
    read_bitfield_data(width, height, input_buffer, output_buffer, stride,
                       red_mask, green_mask, blue_mask, alpha_mask, channels,
                       32);
}

static void RLE_put(int& width, int& height, int& x, int& y,
                    uint8_t* output_buffer, ColourTable& colours,
                    uint8_t index) {
    if(x >= width || y >= height) return;
    output_buffer[((height - 1 - y) * width + x) * 3 + 0] = colours[index].R;
    output_buffer[((height - 1 - y) * width + x) * 3 + 1] = colours[index].G;
    output_buffer[((height - 1 - y) * width + x) * 3 + 2] = colours[index].B;
}

static void read_RLE8_data(int width, int height, RawImage& input_buffer,
                           uint8_t* output_buffer, ColourTable& colours) {
    uint8_t a, b;
    int x = 0, y = 0;
    bool run = true;
    while(run) {
        a = input_buffer.u8();
        b = input_buffer.u8();
        if(a == 0) {
            switch(b) {
                case 0:
                    x = 0;
                    y += 1;
                    break;
                case 1:
                    run = false;
                    break;
                case 2:
                    x += input_buffer.u8();
                    y += input_buffer.u8();
                    break;
                default:
                    for(int i = 0; i < b; ++i, ++x) {
                        uint8_t index = input_buffer.u8();
                        RLE_put(width, height, x, y, output_buffer, colours,
                                index);
                    }
                    if(b & 1) {
                        input_buffer.skip(1);
                    }
                    break;
            }
        }
        else {
            uint8_t index = b;
            for(int i = 0; i < a; ++i, ++x) {
                RLE_put(width, height, x, y, output_buffer, colours, index);
            }
        }
    }
}

static void read_RLE4_data(int width, int height, RawImage& input_buffer,
                           uint8_t* output_buffer, ColourTable& colours) {
    uint8_t a, b;
    int x = 0, y = 0;
    bool run = true;
    while(run) {
        a = input_buffer.u8();
        b = input_buffer.u8();
        if(a == 0) {
            switch(b) {
                case 0:
                    x = 0;
                    y += 1;
                    break;
                case 1:
                    run = false;
                    break;
                case 2:
                    x += input_buffer.u8();
                    y += input_buffer.u8();
                    break;
                default: {
                    uint8_t byte = 0;
                    for(int i = 0; i < b; ++i, ++x) {
                        byte = (i & 1) ? byte : input_buffer.u8();
                        RLE_put(width, height, x, y, output_buffer, colours,
                                (i & 1) ? byte & 0x0F : (byte & 0xF0) >> 4);
                    }
                    if(((b + 1) / 2) & 1) {
                        input_buffer.skip(1);
                    }
                } break;
            }
        }
        else {
            uint8_t byte = b;
            for(int i = 0; i < a; ++i, ++x) {
                RLE_put(width, height, x, y, output_buffer, colours,
                        (i & 1) ? byte & 0x0F : (byte & 0xF0) >> 4);
            }
        }
    }
}

void* read_bmp(const std::string& path, int& width, int& height,
               int& channels) {
    RawImage buffer(path);
    uint8_t* output_buffer = nullptr;
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
    buffer.seek(file_header.bfOffBits);



    width = bitmap_header.biWidth;
    height = bitmap_header.biHeight;
    int stride = ((((width * bitmap_header.biBitCount) + 31) & ~31) >> 3);

    uint32_t red_mask = bitmap_header.bRedMask;
    uint32_t green_mask = bitmap_header.bGreenMask;
    uint32_t blue_mask = bitmap_header.bBlueMask;
    uint32_t alpha_mask = bitmap_header.bAlphaMask;

    // not 3 only in 32 or 16 bpp alpha channel images,
    // only needs to be passed there
    channels = alpha_mask ? 4 : 3;

    output_buffer = (uint8_t*) malloc((size_t) width * height * channels);
    if(output_buffer == nullptr) {
        std::cerr << "Memory allocation failed\n";
        std::terminate();
    }
    memset(output_buffer, 0xFF, (size_t) width * height * channels);

    if(bitmap_header.biCompression == BI_RLE8) {
        read_RLE8_data(width, height, buffer, output_buffer, colours);
        return output_buffer;
    }
    if(bitmap_header.biCompression == BI_RLE4) {
        read_RLE4_data(width, height, buffer, output_buffer, colours);
        return output_buffer;
    }

    switch(bitmap_header.biBitCount) {
        case 1:
            read_1bpp_pixel_data(width, height, buffer, output_buffer, colours,
                                 stride);
            break;
        case 4:
            read_4bpp_pixel_data(width, height, buffer, output_buffer, colours,
                                 stride);
            break;
        case 8:
            read_8bpp_pixel_data(width, height, buffer, output_buffer, colours,
                                 stride);
            break;
        case 16:
            read_16bpp_pixel_data(width, height, buffer, output_buffer, stride,
                                  red_mask, green_mask, blue_mask, alpha_mask,
                                  channels);
            break;
        case 24:
            read_24bpp_pixel_data(width, height, buffer, output_buffer, stride);
            break;
        case 32:
            read_32bpp_pixel_data(width, height, buffer, output_buffer, stride,
                                  red_mask, green_mask, blue_mask, alpha_mask,
                                  channels);
            break;
        default:
            std::cerr << "Invalid bitCount\n";
            std::terminate();
            break;
    }

    return output_buffer;
}