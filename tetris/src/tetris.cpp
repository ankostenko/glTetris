#include "common.cpp"

#include "GameLogic.cpp"
#include "shader.cpp"
#include "Font.cpp"

#include "Timer.cpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glcall(glViewport(0, 0, width, height));
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (!doesCollide(LEFT)) {
            activeFigure.posX -= 1;
        }
    } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!doesCollide(RIGHT)) {
            activeFigure.posX += 1;
        }
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (!doesCollide(BOTTOM)) {
            activeFigure.posY += 1;
        }
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        running = false;
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        rotate_figure(activeFigure.shape);
    }
}

void InitGame() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "Tetris", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create a window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return;
    }
    
    glcall(glViewport(0, 0, width, height));

    glcall(glEnable(GL_CULL_FACE));
    glcall(glEnable(GL_BLEND));
    glcall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Font initialization
    GLuint textShader = initFont();

    float cubeVerticies[] = {
        -20.0f, -20.0f, 0.0f,
         20.0f, -20.0f, 0.0f,
         20.0f,  20.0f, 0.0f,
        -20.0f,  20.0f, 0.0f
    };

    int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    GLuint vao, vbo, ibo;

    glcall(glGenVertexArrays(1, &vao));
    glcall(glGenBuffers(1, &vbo));
    glcall(glGenBuffers(1, &ibo));

    glcall(glBindVertexArray(vao));

    glcall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    glcall(glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerticies), cubeVerticies, GL_STATIC_DRAW));

    glcall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    glcall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    glcall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    glcall(glEnableVertexAttribArray(0));

    // VERTEX SHADER
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexSource = load_shader_source(std::string("src/shaders/vertexShader.vs"))->c_str();
    compile_shader(vertexShader, vertexSource);

    // FRAGMENT SHADER
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentSource = load_shader_source(std::string("src/shaders/fragmentShader.fs"))->c_str();
    compile_shader(fragmentShader, fragmentSource);

    // PROGRAM
    GLuint shaderProgram = glCreateProgram();
    compile_program(shaderProgram, vertexShader, fragmentShader);

    std::srand(std::time(nullptr));
    int colorNumber = rand() % (sizeof(colors) / sizeof(colors[0]));
    int figureNumber = rand() % (sizeof(figures) / sizeof(figures[0]));
    activate_figure(activeFigure.shape, figures[figureNumber]);
    put_border_on_field();

    Timer timer;
    Timer inputTimer;
    Timer msTimer;
    Timer lockTimer;

    std::vector<int> deletedRows;

    while (running) {
        if (glfwWindowShouldClose(window)) {
            running = false;
        }

        if (lockTimer.milliElapsed() > 16.0f) {
            lockTimer.ResetStartTime();
            // Input
            // TODO: controllable input sensibility
            if (inputTimer.milliElapsed() > 60.0f) {
                inputTimer.ResetStartTime();
                processInput(window);
            }

            // Sim
            // TODO: level progression
            float delay = 700.0f - (0.4 * score);
            if (delay < 200.0f) delay = 200.0f;
            if (timer.milliElapsed() > delay) {
                timer.ResetStartTime();
                if (!doesCollide(BOTTOM)) {
                    activeFigure.posY += 1;
                } else {
                    // Check if we lose
                    if (activeFigure.posY < 1) {
                        running = false;
                        ShowGameOver(textShader, window);
                        break;
                    }

                    put_stone_blocks();
                    save_field_state(HEIGHT, WIDTH);
                    activeFigure.posY = 0;
                    activeFigure.posX = 4;

                    colorNumber = rand() % (sizeof(colors) / sizeof(colors[0]));
                    figureNumber = rand() % (sizeof(figures) / sizeof(figures[0]));
                    activate_figure(activeFigure.shape, figures[figureNumber]);

                    deletedRows = delete_filled_rows(window, vao, ibo, shaderProgram);
                    score += 10;
                }
            }

            // Render
            glcall(glClearColor(0.172, 0.243, 0.313, 1.0f));
            glcall(glClear(GL_COLOR_BUFFER_BIT));

            // Active Figure
            for (int row = 0; row < 3; row++) {
                for (int col = 0; col < 3; col++) {
                    if (get_block_at(activeFigure.shape, row, col, 3) == 'X') {
                        DrawQuad(vao, ibo, shaderProgram, QUAD_SIZE + (col + activeFigure.posX) * QUAD_SIZE, QUAD_SIZE * (HEIGHT - (activeFigure.posY + row)), colors[colorNumber]);
                    }
                }
            }

            // Redraw field
            for (int row = 0; row < HEIGHT; row++) {
                for (int col = 0; col < WIDTH; col++) {
                    if (get_block_at(field, row, col, WIDTH) == '0') {
                        for (int ind : deletedRows) {
                        }
                            DrawQuad(vao, ibo, shaderProgram, QUAD_SIZE + col * QUAD_SIZE, (HEIGHT - row - 1) * QUAD_SIZE + QUAD_SIZE, glm::vec4(0.203, 0.596, 0.858, 1.0f));
                    }
                }
            }

            // Text Rendering
            std::ostringstream oss;
            oss << "Score: " << score;
            RenderText(textShader, oss.str(), width - 10 * QUAD_SIZE, height - 2 * QUAD_SIZE, 1.0f, glm::vec3(0.145, 0.564, 0.658));
            std::ostringstream frameMS;
            frameMS << msTimer.milliElapsed() << " ms/frame";
            RenderText(textShader, frameMS.str(), width - 10 * QUAD_SIZE, height - 3 * QUAD_SIZE, 1.0f, glm::vec3(0.145, 0.564, 0.658));
            std::ostringstream framesPerSec;
            framesPerSec << 1000.0f / msTimer.milliElapsed() << " frame/sec";
            RenderText(textShader, framesPerSec.str(), width - 10 * QUAD_SIZE, height - 4 * QUAD_SIZE, 1.0f, glm::vec3(0.145, 0.564, 0.658));

            msTimer.ResetStartTime();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    glfwTerminate();
}

int main() {
    InitGame();
    return 0;
}