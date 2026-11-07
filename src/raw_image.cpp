#include "raw_image.hpp"

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>

RawImage::RawImage(const std::string& path) { read_image(path); }

void RawImage::read_image(const std::string& path) {
    std::ifstream image(path, std::ios::binary | std::ios::ate);
    if(!image) {
        std::cerr << "Image can not be opened!\n";
        std::terminate();
    }
    size_t size = image.tellg();
    image.seekg(0);
    if(data) {
        delete_data();
    }
    data = (uint8_t*) malloc(size);
    if(!data) {
        std::cerr << "Allocation error\n";
        std::terminate();
    }
    if(!image.read((char*) data, size)) {
        std::cerr << "Image could not be read\n";
        std::terminate();
    }
    buffer = data;
    buffer_end = data + size;
}

void RawImage::delete_data() {
    free(data);
    data = nullptr;
    buffer = nullptr;
    buffer_end = nullptr;
}

RawImage::~RawImage() { delete_data(); }

uint8_t RawImage::u8() {
    if(buffer_end - buffer < 1) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    uint8_t value = *((uint8_t*) buffer);
    buffer += 1;
    return value;
}

uint16_t RawImage::u16le() {
    if(buffer_end - buffer < 2) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    uint16_t value = ((uint16_t) buffer[0]) | ((uint16_t) buffer[1]) << 8;
    buffer += 2;
    return value;
}

uint16_t RawImage::u16be() {
    if(buffer_end - buffer < 2) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    uint16_t value = ((uint16_t) buffer[0]) << 8 | ((uint16_t) buffer[1]);
    buffer += 2;
    return value;
}

uint32_t RawImage::u32le() {
    if(buffer_end - buffer < 4) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    uint32_t value = ((uint32_t) buffer[0]) | ((uint32_t) buffer[1]) << 8 |
                     ((uint32_t) buffer[2]) << 16 |
                     ((uint32_t) buffer[3]) << 24;
    buffer += 4;
    return value;
}

uint32_t RawImage::u32be() {
    if(buffer_end - buffer < 4) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    uint32_t value = ((uint32_t) buffer[0]) << 24 |
                     ((uint32_t) buffer[1]) << 16 |
                     ((uint32_t) buffer[2]) << 8 | ((uint32_t) buffer[3]);
    buffer += 4;
    return value;
}

int8_t RawImage::i8() {
    if(buffer_end - buffer < 1) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    int8_t value = *((int8_t*) buffer);
    buffer += 1;
    return value;
}

int16_t RawImage::i16le() {
    if(buffer_end - buffer < 2) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    int16_t value = ((int16_t) buffer[0]) | ((int16_t) buffer[1]) << 8;
    buffer += 2;
    return value;
}

int16_t RawImage::i16be() {
    if(buffer_end - buffer < 2) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    int16_t value = ((int16_t) buffer[0]) << 8 | ((int16_t) buffer[1]);
    buffer += 2;
    return value;
}

int32_t RawImage::i32le() {
    if(buffer_end - buffer < 4) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    int32_t value = ((int32_t) buffer[0]) | ((int32_t) buffer[1]) << 8 |
                    ((int32_t) buffer[2]) << 16 | ((int32_t) buffer[3]) << 24;
    buffer += 4;
    return value;
}

int32_t RawImage::i32be() {
    if(buffer_end - buffer < 4) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    int32_t value = ((int32_t) buffer[0]) << 24 | ((int32_t) buffer[1]) << 16 |
                    ((int32_t) buffer[2]) << 8 | ((int32_t) buffer[3]);
    buffer += 4;
    return value;
}

void RawImage::skip(size_t count) {
    if(buffer_end - buffer < count) {
        // TODO log
        std::cout << "Not enough bytes\n";
        std::terminate();
    }
    buffer += count;
}

void RawImage::seek(size_t position) {
    buffer = data + position < buffer_end ? data + position : buffer;
}