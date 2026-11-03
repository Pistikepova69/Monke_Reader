
#ifndef TEXTURE_HPP
#define TEXTURE_HPP
 
#include <glad/gl.h>
 
#include <string>
 
class texture2D {
    unsigned int id{};
    int width{}, height{};
 
  public:
    explicit texture2D(const std::string& path);
    texture2D(const texture2D&) = delete;
    texture2D& operator=(const texture2D&) = delete;
    texture2D(texture2D&& other) noexcept;
    texture2D& operator=(texture2D&& other) noexcept;
    ~texture2D();
 
    void load(const std::string& path);
    void unload();
    void use() const;
    void activate(GLenum unit = GL_TEXTURE0) const;
 
    int get_width() const { return width; }
    int get_height() const { return height; }
};
 
#endif  // TEXTURE_HPP
