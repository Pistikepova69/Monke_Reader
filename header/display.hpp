#ifndef DISPLAY_HPP
#define DISPLAY_HPP
 
#include <string>
 
struct GLFWwindow;
 
// Owns GLFW, the window and the OpenGL context (4.6 core, glad loaded).
// Create it before any other OpenGL object and destroy it after them.
// The window starts hidden so it can be sized first: call show() when ready.
class Window {
    GLFWwindow* handle{};
 
  public:
    Window(int width, int height, const std::string& title);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    ~Window();
 
    bool should_close() const;
    void swap_and_poll() const;  // swap buffers, then process events
 
    // Resize the window and lock its aspect ratio to width:height.
    void set_content_size(int width, int height);
    void show() const;
};
 
// Fullscreen quad (NDC -1..1) with position + uv, ready for a texture.
class Quad {
    unsigned int VAO{}, VBO{}, EBO{};
 
  public:
    Quad();
    Quad(const Quad&) = delete;
    Quad& operator=(const Quad&) = delete;
    ~Quad();
 
    void draw() const;
};
 
#endif  // DISPLAY_HPP
