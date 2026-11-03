#ifndef INPUT_HPP
#define INPUT_HPP


#include <cstddef>
#include <cstdint>
#include <string>

struct RawImage {
    uint8_t *data = nullptr, *buffer = nullptr, *buffer_end = nullptr;
    RawImage(const std::string& path);
    RawImage() = default;
    void read_image(const std::string& path);
    void delete_data();
    ~RawImage();
    uint8_t u8();
    uint16_t u16le();
    uint16_t u16be();
    uint32_t u32le();
    uint32_t u32be();
    uint64_t u64le() = delete;
    uint64_t u64be() = delete;
    void skip(size_t count);
};

#endif  // INPUT_HPP