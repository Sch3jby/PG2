#include "app.hpp"
#include <iostream>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>

bool App::init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    // Create a window
    window = glfwCreateWindow(800, 600, "OpenGL Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return false;
    }

    // Check for DSA support
    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "No Direct State Access support" << std::endl;
        glfwTerminate();
        return false;
    }

    // Set key callback for color changes
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->key_callback(window, key, scancode, action, mods);
        }
        });

    // Initialize assets (shaders, buffers, etc.)
    init_assets();

    return true;
}

void App::init_assets() {
    //
    // Initialize pipeline: compile, link and use shaders
    //

    //SHADERS - define & compile & link
    const char* vertex_shader =
        "#version 460 core\n"
        "in vec3 attribute_Position;"
        "void main() {"
        "  gl_Position = vec4(attribute_Position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 460 core\n"
        "uniform vec4 uniform_Color;"
        "out vec4 FragColor;"
        "void main() {"
        "  FragColor = uniform_Color;"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    // Check for vertex shader compilation errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    // Check for fragment shader compilation errors
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    shader_prog_ID = glCreateProgram();
    glAttachShader(shader_prog_ID, fs);
    glAttachShader(shader_prog_ID, vs);
    glLinkProgram(shader_prog_ID);

    // Check for shader program linking errors
    glGetProgramiv(shader_prog_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_prog_ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Now we can delete shader parts
    glDetachShader(shader_prog_ID, fs);
    glDetachShader(shader_prog_ID, vs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 
    // Create and load data into GPU using OpenGL DSA (Direct State Access)
    //

    // Create VAO + data description
    glCreateVertexArrays(1, &VAO_ID);

    GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");

    glEnableVertexArrayAttrib(VAO_ID, position_attrib_location);
    glVertexArrayAttribFormat(VAO_ID, position_attrib_location, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position));
    glVertexArrayAttribBinding(VAO_ID, position_attrib_location, 0);

    // Create and fill data
    glCreateBuffers(1, &VBO_ID);
    glNamedBufferData(VBO_ID, triangle_vertices.size() * sizeof(vertex), triangle_vertices.data(), GL_STATIC_DRAW);

    // Connect together
    glVertexArrayVertexBuffer(VAO_ID, 0, VBO_ID, 0, sizeof(vertex));
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // Change color based on key presses
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        const float delta = 0.1f;

        // Red color
        if (key == GLFW_KEY_R) {
            r = (r >= 1.0f) ? 0.0f : r + delta;
        }

        // Green color
        if (key == GLFW_KEY_G) {
            g = (g >= 1.0f) ? 0.0f : g + delta;
        }

        // Blue color
        if (key == GLFW_KEY_B) {
            b = (b >= 1.0f) ? 0.0f : b + delta;
        }

        // Reset to red
        if (key == GLFW_KEY_0) {
            r = 1.0f;
            g = 0.0f;
            b = 0.0f;
        }
    }
}

void App::run() {
    // Start time for color animation
    auto start_time = std::chrono::high_resolution_clock::now();

    // Activate shader program
    glUseProgram(shader_prog_ID);

    // Get uniform location in GPU program
    GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
    if (uniform_color_location == -1) {
        std::cerr << "Uniform location is not found in active shader program. Did you forget to activate it?\n";
    }

    bool animate_color = false;

    // FPS counter variables
    int frameCount = 0;
    double lastTime = glfwGetTime();
    double currentTime;
    double fps = 0.0;
    bool vsync_enabled = false;

    // Toggle VSync initially off for maximum FPS
    glfwSwapInterval(0);

    while (!glfwWindowShouldClose(window)) {
        // Calculate time elapsed for potential color animation
        auto current_animation_time = std::chrono::high_resolution_clock::now();
        auto time_elapsed = std::chrono::duration<float>(current_animation_time - start_time).count();

        // Toggle color animation with spacebar
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            animate_color = !animate_color;
            // Small delay to prevent multiple toggles
            glfwWaitEventsTimeout(0.2);
        }

        // Toggle VSync with F12
        if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS) {
            vsync_enabled = !vsync_enabled;
            glfwSwapInterval(vsync_enabled ? 1 : 0);
            std::cout << "VSync: " << (vsync_enabled ? "ON" : "OFF") << std::endl;
            // Small delay to prevent multiple toggles
            glfwWaitEventsTimeout(0.2);
        }

        // Update color based on time if animation is enabled
        if (animate_color) {
            r = (std::sin(time_elapsed) + 1.0f) / 2.0f;
            g = (std::sin(time_elapsed + 2.0f * 3.14159f / 3.0f) + 1.0f) / 2.0f;
            b = (std::sin(time_elapsed + 4.0f * 3.14159f / 3.0f) + 1.0f) / 2.0f;
        }

        // Get cursor position for optional color control
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // If right mouse button is pressed, use cursor position to change color
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            // Normalize cursor position to [0, 1] range
            r = static_cast<float>(xpos) / width;
            g = static_cast<float>(ypos) / height;
            b = 1.0f - (r + g) / 2.0f;  // Make sure the third component changes too
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set uniform parameter for shader
        glUniform4f(uniform_color_location, r, g, b, a);

        // Bind 3D object data
        glBindVertexArray(VAO_ID);

        // Draw all VAO data
        glDrawArrays(GL_TRIANGLES, 0, triangle_vertices.size());

        // Update FPS counter
        frameCount++;
        currentTime = glfwGetTime();

        // If a second has passed
        if (currentTime - lastTime >= 1.0) {
            // Calculate the FPS
            fps = static_cast<double>(frameCount) / (currentTime - lastTime);

            // Update window title with FPS
            std::stringstream ss;
            ss << "OpenGL Triangle | FPS: " << std::fixed << std::setprecision(1) << fps
                << " | VSync: " << (vsync_enabled ? "ON" : "OFF")
                << " | R:" << std::setprecision(2) << r << " G:" << g << " B:" << b;
            glfwSetWindowTitle(window, ss.str().c_str());

            // Reset counters
            frameCount = 0;
            lastTime = currentTime;
        }

        // Poll events, call callbacks, swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}

App::~App() {
    // Cleanup GL data
    glDeleteProgram(shader_prog_ID);
    glDeleteBuffers(1, &VBO_ID);
    glDeleteVertexArrays(1, &VAO_ID);

    // Terminate GLFW
    glfwTerminate();
}