#ifndef BMP_HPP
#define BMP_HPP
#include <cstddef>
#include <cstdint>
#include <string>

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

struct BitmapInfoHeader {
    uint32_t biSize;          // 40!
    int32_t biWidth;          // dimesnion of image
    int32_t biHeight;         // dimesnion of image
    uint16_t biPlanes;        // 1!
    uint16_t biBitCount;      // bits per pixel (1, 4, 8, 16, 24, 32)
    uint32_t biCompression;   // currently only BI_RGB maybe more?
    uint32_t biSizeImage;     // irrelevant
    int32_t biXPelsPerMeter;  // irrelevant
    int32_t biYPelsPerMeter;  // irrelevant
    uint32_t biClrUsed;       // colour table size
    uint32_t biClrImportant;  // irrelevant

    void read_header(RawImage& buffer);
};

struct BitmapHeader {
    enum valid_versions : uint8_t { Info = 1, V4 = 4, V5 = 5, None = 0 };

    valid_versions version = None;


    uint32_t biSize;          // 40 or 108 or 124!
    int32_t biWidth;          // dimesnion of image
    int32_t biHeight;         // dimesnion of image
    uint16_t biPlanes;        // 1!
    uint16_t biBitCount;      // bits per pixel (1, 4, 8, 16, 24, 32)
    uint32_t biCompression;   // currently only BI_RGB maybe more?
    uint32_t biSizeImage;     // irrelevant
    int32_t biXPelsPerMeter;  // irrelevant
    int32_t biYPelsPerMeter;  // irrelevant
    uint32_t biClrUsed;       // colour table size
    uint32_t biClrImportant;  // irrelevant
    // info header until this point
    uint32_t bRedMask;  // bitmasks to tead color data, only if bpp = (16,  32)
    uint32_t bGreenMask;  // bitmasks to tead color data, onli if bpp = (16, 32)
    uint32_t bBlueMask;   // bitmasks to tead color data, onli if bpp = (16, 32)
    uint32_t bAlphaMask;  // bitmasks to tead color data, onli if bpp = (16, 32)
    uint32_t bCSType;     //  0x73524742 or 0x57696E20
    uint8_t bEndpoints[36];  // not the real field type, skip, not relevant
    uint32_t bGammaRed;      // can be ignored
    uint32_t bGammaGreen;    // can be ignored
    uint32_t bGammaBlue;     // can be ignored
    // V4 until this point
    uint32_t bIntent;       // irrelevant
    uint32_t bProfileData;  // irrelevant
    uint32_t bProfileSize;  // irrelevant
    uint32_t bReserved;     // irrelevant (0!)
    void read_header(RawImage& buffer);
};

void* read_bmp(const std::string& path, int& width, int& height, int& channels);

#endif