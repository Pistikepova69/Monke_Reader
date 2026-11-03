// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include "display.hpp"
 
#include <iostream>
 
static void on_resize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}
 
Window::Window(int width, int height, const std::string& title) {
    if(!glfwInit()) {
        std::cerr << ("glfwInit failed\n");
        std::terminate();
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // shown later via show()
 
    handle = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if(!handle) {
        glfwTerminate();
        std::cerr << ("Failed to create window\n");
        std::terminate();
    }
    glfwMakeContextCurrent(handle);
    glfwSwapInterval(1);
 
    if(gladLoadGL(glfwGetProcAddress) == 0) {
        glfwDestroyWindow(handle);
        glfwTerminate();
        std::cerr << ("Failed to initialize OpenGL context\n");
        std::terminate();
    }
 
    glfwSetFramebufferSizeCallback(handle, on_resize);
    int fb_w, fb_h;
    glfwGetFramebufferSize(handle, &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);
 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
 
Window::~Window() {
    glfwDestroyWindow(handle);
    glfwTerminate();
}
 
bool Window::should_close() const { return glfwWindowShouldClose(handle); }
 
void Window::swap_and_poll() const {
    glfwSwapBuffers(handle);
    glfwPollEvents();
}
 
void Window::set_content_size(int width, int height) {
    glfwSetWindowSize(handle, width, height);
    glfwSetWindowAspectRatio(handle, width, height);
}
 
void Window::show() const { glfwShowWindow(handle); }
 
Quad::Quad() {
    float vertices[] = {
        // pos          // uv
        -1.0f, 1.0f,  0.0f, 0.0f,  // top-left
        -1.0f, -1.0f, 0.0f, 1.0f,  // bottom-left
        1.0f,  -1.0f, 1.0f, 1.0f,  // bottom-right
        1.0f,  1.0f,  1.0f, 0.0f   // top-right
    };
    unsigned int indices[] = {0, 1, 2, 0, 2, 3};
 
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
 
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);
 
    // location 0: position (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*) 0);
    glEnableVertexAttribArray(0);
    // location 1: texcoord (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);
 
    glBindVertexArray(0);
}
 
Quad::~Quad() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
 
void Quad::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
