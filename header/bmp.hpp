#ifndef BMP_HPP
#define BMP_HPP

#include <string>

void* read_bmp(const std::string& path, int& width, int& height, int& channels);

#endif