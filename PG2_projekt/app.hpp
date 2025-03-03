#pragma once

#include <GL/glew.h> 
#include <GL/wglew.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "assets.hpp"

class App {
private:
    // GLFW window
    GLFWwindow* window{ nullptr };

    // GL stuff
    GLuint shader_prog_ID{ 0 };
    GLuint VBO_ID{ 0 };
    GLuint VAO_ID{ 0 };

    // Color components
    GLfloat r{ 1.0f }, g{ 0.0f }, b{ 0.0f }, a{ 1.0f };

    // Triangle vertices
    std::vector<vertex> triangle_vertices = {
        {{0.0f,  0.5f,  0.0f}},
        {{0.5f, -0.5f,  0.0f}},
        {{-0.5f, -0.5f,  0.0f}}
    };

    void init_assets();
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    App() = default;
    ~App();

    bool init();
    void run();
};