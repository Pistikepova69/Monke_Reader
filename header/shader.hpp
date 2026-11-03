
#ifndef SHADER_HPP
#define SHADER_HPP
 
#include <string>
 
class Shader {
    unsigned int id{};
 
  public:
    Shader(const std::string& vertex_shader_path,
           const std::string& fragment_shader_path);
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    ~Shader();
 
    void use() const;
    void set_int(const std::string& uniform_name, int value) const;
};
 
#endif  // SHADER_HPP
