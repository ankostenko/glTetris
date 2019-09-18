#include <fstream>
#include <string>
#include <unordered_map>

static std::unordered_map<std::string, GLint> uniformLocationCache;

std::string *load_shader_source(std::string fileName) {
    std::ifstream ifs(fileName);
    std::string *content = new std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return content;
}

void compile_shader(GLuint shader, const char *source) {
    glcall(glShaderSource(shader, 1, &source, NULL));
    glcall(glCompileShader(shader));

    int success;
    char infoLog[512];
    glcall(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader compilation failed!" << infoLog << std::endl;
    }
}

void compile_program(GLuint program, GLuint vShader, GLuint fShader) {
    glcall(glAttachShader(program, vShader));
    glcall(glAttachShader(program, fShader));
    glcall(glLinkProgram(program));

    int success;
    char infoLog[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Program compilation failed!" << infoLog << std::endl;
    }

    glcall(glDeleteShader(vShader));
    glcall(glDeleteShader(fShader));
}

int GetUniformLocation(unsigned int program, const char *un_name) {
    int location;

    if (uniformLocationCache.find(un_name) != uniformLocationCache.end()) {
        return uniformLocationCache[un_name];
    }

    glcall(location = glGetUniformLocation(program, un_name));
    uniformLocationCache[un_name] = location;

    if (location == -1) {
        std::cout << "Warning: uniform " << un_name << " doesn't exist!" << std::endl;
    }
    return location;
}

void SetUniformMat4f(unsigned int program, const char *un_name, glm::mat4 &matrix) {
    glcall(glUniformMatrix4fv(GetUniformLocation(program, un_name), 1, GL_FALSE, &matrix[0][0]));
}


void SetUniform3f(unsigned int program, const char *un_name, glm::vec3 vec) {
    glcall(glUniform3f(GetUniformLocation(program, un_name), vec.r, vec.g, vec.b));
}

void SetUniform4f(unsigned int program, const char *un_name, glm::vec4 vec) {
    glcall(glUniform4f(GetUniformLocation(program, un_name), vec.r, vec.g, vec.b, vec.a));
}

struct IndexBuffer {
    unsigned int ID;
    int count;
};

struct VertexArray {
    unsigned int ID;
};

void Render(VertexArray va, IndexBuffer ib, unsigned int shaderProgram) {
    glcall(glBindVertexArray(va.ID));
    glcall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.ID));

    glcall(glDrawElements(GL_TRIANGLES, ib.count, GL_UNSIGNED_INT, nullptr));
}

const float cubeWidth = 150.0f;
const float cubeHeight = 150.0f;

void DrawQuad(unsigned int vao, unsigned int ibo, unsigned int program, float posx, float posy, glm::vec4 color) {
    glm::mat4 proj = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(posx, posy, 0.0f));

    glm::mat4 mvp = proj * view * model;

    glUseProgram(program);
    SetUniformMat4f(program, "u_MVP", mvp);
    SetUniform4f(program, "u_QuadColor", color);

    VertexArray va = { vao };
    IndexBuffer ib = { ibo, 6 };
    Render(va, ib, program);
}