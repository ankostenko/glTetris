#include <ft2build.h>
#include FT_FREETYPE_H

static GLuint vertexArray;
static GLuint vertexBuffer;

struct Character {
    GLuint      TextureID;
    glm::ivec2  Size;
    glm::ivec2  Bearing;
    GLuint      Advance;
};

std::map<GLchar, Character> characters;

GLuint initFont() {
    // Shader compilation
    // VERTEX SHADER
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexSource = load_shader_source(std::string("src/shaders/fontVertex.shader"))->c_str();
    compile_shader(vertexShader, vertexSource);

    // FRAGMENT SHADER
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentSource = load_shader_source(std::string("src/shaders/fontFragment.shader"))->c_str();
    compile_shader(fragmentShader, fragmentSource);

    // PROGRAM
    GLuint shaderProgram = glCreateProgram();
    compile_program(shaderProgram, vertexShader, fragmentShader);

    FT_Library ft;

    if (FT_Init_FreeType(&ft)) {
        std::cout << "Couldn't init FreeType library" << std::endl;
        return shaderProgram;
    }

    FT_Face face;
    if (FT_New_Face(ft, "src/fonts/bauh/bauhs93.ttf", 0, &face)) {
        std::cout << "Failed to load font" << std::endl;
        return shaderProgram;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    glcall(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    for (GLubyte c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "Failed to load glyph " << c << std::endl;
            continue;
        }

        GLuint texture;
        glcall(glGenTextures(1, &texture));
        glcall(glBindTexture(GL_TEXTURE_2D, texture));

        glcall(glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        ));

        glcall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        glcall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        glcall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glcall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };

        characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glcall(glBindTexture(GL_TEXTURE_2D, 0));

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glm::mat4 projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    glcall(glUseProgram(shaderProgram));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glcall(glGenVertexArrays(1, &vertexArray));
    glcall(glGenBuffers(1, &vertexBuffer));
    glcall(glBindVertexArray(vertexArray));
    glcall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
    glcall(glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * 6 * 4, NULL, GL_DYNAMIC_DRAW));
    glcall(glEnableVertexAttribArray(0));
    glcall(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0));
    glcall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    glcall(glBindVertexArray(0));

    return shaderProgram;
}

void RenderText(GLuint shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
    glcall(glUseProgram(shader));

    SetUniform3f(shader, "textColor", color);
    glcall(glActiveTexture(GL_TEXTURE0));
    glcall(glBindVertexArray(vertexArray));

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f },
        };

        glcall(glBindTexture(GL_TEXTURE_2D, ch.TextureID));
        glcall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
        glcall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
        glcall(glBindBuffer(GL_ARRAY_BUFFER, 0));

        glcall(glDrawArrays(GL_TRIANGLES, 0, 6));

        x += (ch.Advance >> 6) * scale;
    }

    glcall(glBindVertexArray(0));
    glcall(glBindTexture(GL_TEXTURE_2D, 0));
}