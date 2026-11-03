// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

static std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if(!in.is_open()) {
        std::cerr << ("Cannot open shader file: " + path + "\n");
        std::terminate();
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static unsigned int compile(GLenum type, const std::string& path) {
    std::string source = read_file(path);
    const char* cstr = source.c_str();

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if(ok != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof log, nullptr, log);
        glDeleteShader(shader);
        std::cerr << ("Shader compile error in " + path + ":\n" + log + "\n");
        std::terminate();
    }
    return shader;
}

Shader::Shader(const std::string& vertex_shader_path,
               const std::string& fragment_shader_path) {
    unsigned int vertex = compile(GL_VERTEX_SHADER, vertex_shader_path);
    unsigned int fragment = compile(GL_FRAGMENT_SHADER, fragment_shader_path);

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint ok = GL_FALSE;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if(ok != GL_TRUE) {
        char log[1024];
        glGetProgramInfoLog(id, sizeof log, nullptr, log);
        glDeleteProgram(id);
        id = 0;
        std::cerr << (std::string("Shader link error:\n") + log + "\n");
        std::terminate();
    }
}

Shader::~Shader() {
    if(id != 0 && glfwGetCurrentContext() != nullptr) {
        glDeleteProgram(id);
    }
}

void Shader::use() const { glUseProgram(id); }

void Shader::set_int(const std::string& uniform_name, int value) const {
    glUniform1i(glGetUniformLocation(id, uniform_name.c_str()), value);
}
