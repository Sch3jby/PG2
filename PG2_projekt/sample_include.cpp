/*//
// Be careful! Includes are (in general) position dependent!
//

// C++ (in any order) 
#include <iostream>
#include <chrono>

// OpenCV (image processing and camera support) 
// does not depend on GL
#include <opencv2\opencv.hpp>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h> 
// WGLEW = Windows GL Extension Wrangler (change for different platform) 
// platform specific functions (in this case Windows)
#include <GL/wglew.h> 

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW must be first.
#include <GLFW/glfw3.h>

// OpenGL math
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL    // allow experimental - needed for glm::to_string()
#include <glm/gtx/string_cast.hpp> // glm::to_string()
#include <glm/gtc/type_ptr.hpp>    // access to raw internal data

// JSON support - for config files etc.
#include <nlohmann/json.hpp>

*/