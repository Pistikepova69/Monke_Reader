cmake_minimum_required(VERSION 4.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4)

FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG v2.0.8)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master)

project(Image_Reader LANGUAGES C CXX)
add_executable(Image_Reader)

FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
  message("Fetching glad")
  FetchContent_MakeAvailable(glad)

  add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
  glad_add_library(
    glad
    REPRODUCIBLE
    EXCLUDE_FROM_ALL
    LOADER
    API
    gl:core=4.6
    EXTENSIONS
    GL_ARB_bindless_texture
    GL_EXT_texture_compression_s3tc)
endif()

FetchContent_MakeAvailable(glfw stb)

target_sources(Image_Reader PRIVATE src/main.cpp src/shader.cpp src/texture.cpp
                                    src/display.cpp)
target_include_directories(Image_Reader PRIVATE header)
target_include_directories(Image_Reader PRIVATE header ${stb_SOURCE_DIR})
find_package(OpenGL REQUIRED)
target_link_libraries(Image_Reader PRIVATE OpenGL::GL glad glfw)
target_compile_definitions(
  Image_Reader PRIVATE SHADER="${CMAKE_SOURCE_DIR}/shaders/"
                       IMAGES="${CMAKE_SOURCE_DIR}/test_images/")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
