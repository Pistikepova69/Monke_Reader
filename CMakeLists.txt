cmake_minimum_required(VERSION 4.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(Image_Reader LANGUAGES CXX)
add_executable(Image_Reader)

target_sources(Image_Reader PRIVATE src/main.cpp)
target_include_directories(Image_Reader PRIVATE header)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)