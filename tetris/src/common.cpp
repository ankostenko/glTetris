#include <glad/glad.h>
#include <GLFW3/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <sstream>

#include <thread>
#include <chrono>
#include <conio.h>
#include <ctime>
#include <vector>

#include <map>

#if DEBUG_MODE

void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char *funcName, const char *filename, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OPENGL ERROR] " << funcName << " " << filename << " " << line << std::endl;
        return false;
    }
    return true;
}

#define ASSERT(x) if (!(x)) __debugbreak(); 
#define glcall(x) GLClearError(); \
    (x); \
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

#else
#define glcall(x) (x)
#endif