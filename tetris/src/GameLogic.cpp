#define QUAD_SIZE 41.0f

#define get_block_at(type, row, column, width) (type)[(row) * (width) + (column)]
#define WIDTH 16
#define HEIGHT 16

// TODO: Move somewhere else                      // For a text data
const int width = WIDTH * QUAD_SIZE + QUAD_SIZE + 10 * QUAD_SIZE;
const int height = HEIGHT * QUAD_SIZE + QUAD_SIZE;

bool running = true;

struct ActiveFigure {
    char shape[9];
    int posX, posY;
    int colorNumber;


    ActiveFigure(int x, int y)
        : posX(x), posY(y) {

    }
};


const char *figures[] = {
        " X "
        " X "
        " X ",
        "XX "
        " X "
        " X ",
        "X  "
        "XX "
        "X  ",
        "XX "
        "XX "
        "   ",
        "XX "
        " XX"
        "   ",
        " XX"
        "XX "
        "   ",
        " XX"
        " X "
        " X ",
};

glm::vec4 colors[] = {
    glm::vec4(0.203, 0.596, 0.858, 1.0f), // Blue
    glm::vec4(0.752, 0.223, 0.168, 1.0f), // Red
    glm::vec4(0.945, 0.768, 0.058, 1.0f), // Yellow
    glm::vec4(0.607, 0.349, 0.713, 1.0f), // Violet
    glm::vec4(0.901, 0.494, 0.133, 1.0f), // Orange
};

char field[HEIGHT * WIDTH];
char oldFieldState[WIDTH * HEIGHT];
ActiveFigure activeFigure(4, 0);

// Score stuff
int score;

bool isValidToRotate() {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            if (get_block_at(field, activeFigure.posY + row, activeFigure.posX + col, WIDTH) >= '0') {
                return false;
            }
        }
    }
    return true;
}

void rotate_figure(char *figure) {
    if (!isValidToRotate()) {
        return;
    }

    char old_state[9];

    memcpy(old_state, figure, 9);

    // corners
    figure[0 * 3 + 0] = old_state[2 * 3 + 0];
    figure[0 * 3 + 2] = old_state[0 * 3 + 0];
    figure[2 * 3 + 2] = old_state[0 * 3 + 2];
    figure[2 * 3 + 0] = old_state[2 * 3 + 2];

    // cross
    figure[0 * 3 + 1] = old_state[1 * 3 + 0];
    figure[1 * 3 + 2] = old_state[0 * 3 + 1];
    figure[2 * 3 + 1] = old_state[1 * 3 + 2];
    figure[1 * 3 + 0] = old_state[2 * 3 + 1];
}

void activate_figure(char *active_figure, const char *next_figure) {
    for (int i = 0; i < 9; i++) {
        active_figure[i] = next_figure[i];
    }
}

void update_field(int row, int column, char symbol) {
    field[row * WIDTH + column] = symbol;
}

void put_border_on_field() {
    for (int row = 0; row < HEIGHT; row++) {
        for (int column = 0; column < WIDTH; column++) {
            if (column == 0 || column == WIDTH - 1 || row == HEIGHT - 1) {
                update_field(row, column, '0');
            }
        }
    }
}

void save_field_state(int row_to_limit, int col_to_limit) {
    for (int row = 0; row < row_to_limit; row++) {
        for (int col = 0; col < col_to_limit; col++) {
            get_block_at(oldFieldState, row, col, WIDTH) = get_block_at(field, row, col, WIDTH);
        }
    }
}

void restore_field_state() {
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            if (get_block_at(field, row, col, WIDTH) != 'X' && get_block_at(oldFieldState, row, col, WIDTH) != 'X') {
                get_block_at(field, row, col, WIDTH) = get_block_at(oldFieldState, row, col, WIDTH);
            }
        }
    }
}

void put_stone_blocks() {
    for (int row = 0; row < 3; row++) {
        for (int column = 0; column < 3; column++) {
            if (get_block_at(activeFigure.shape, row, column, 3) == 'X') {
                get_block_at(field, activeFigure.posY + row, activeFigure.posX + column, WIDTH) = '0' + activeFigure.colorNumber;
            }
        }
    }
}

enum Side {
    LEFT,
    RIGHT,
    BOTTOM,
};

// FIX: Ã figure can go through objects if it interact with hat thing
bool doesCollide(Side side) {
    switch (side) {
        case LEFT: {
            for (int row = 0; row < 3; row++) {
                for (int column = 0; column < 3; column++) {
                    if (get_block_at(activeFigure.shape, row, column, 3) == 'X') {
                        if (get_block_at(field, activeFigure.posY + row, activeFigure.posX + column - 1, WIDTH) >= '0') {
                            return true;
                        }
                    }
                }
            }
        } break;
        case RIGHT: {
            for (int row = 0, column = 2; row < 3; row++) {
                for (int column = 0; column < 3; column++) {
                    if (activeFigure.shape[row * 3 + column] == 'X') {
                        if (get_block_at(field, activeFigure.posY + row, activeFigure.posX + column + 1, WIDTH) >= '0') {
                            return true;
                        }
                    }
                }
            }
        } break;
        case BOTTOM: {
            for (int row = 0; row < 3; row++) {
                for (int column = 0; column < 3; column++) {
                    if (activeFigure.shape[row * 3 + column] == 'X') {
                        // Field is decreased by 1
                        if (get_block_at(field, activeFigure.posY + row + 1, activeFigure.posX + column, WIDTH) >= '0') {
                            return true;
                        }
                    }
                }
            }
        } break;
    }

    return false;
}

bool check_row_deletion(int row) {
    int row_sum = 0;
    for (int col = 1; col < WIDTH - 1; col++) {
        if (get_block_at(field, row, col, WIDTH) > '0') {
            row_sum++;
        }
    }

    return (row_sum == (WIDTH - 2)) ? true : false;
}

void DrawQuad(unsigned int vao, unsigned int ibo, unsigned int program, float posx, float posy, glm::vec4 color);

static std::vector<int> deletedRows;

std::vector<int>& delete_filled_rows(GLFWwindow *window, GLuint va, GLuint ib, GLuint shader) {
    deletedRows.clear();
    deletedRows.reserve(2);
    for (int row = 0; row < HEIGHT - 1; row++) {
        if (check_row_deletion(row)) {
            deletedRows.push_back(row);
            for (int col = 1; col < WIDTH - 1; col++) {
                update_field(row, col, ' ');
            }

            for (int col = 1; col < WIDTH - 1; col++) {
                for (int ind : deletedRows) {
                    if (row == ind) {
                        DrawQuad(va, ib, shader, QUAD_SIZE + col * QUAD_SIZE, (HEIGHT - row - 1) * QUAD_SIZE + QUAD_SIZE, glm::vec4(0.925, 0.941, 0.945, 1.0f));
                    }
                }
            }
            glfwSwapBuffers(window);
            std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(50));
            
            save_field_state(HEIGHT, WIDTH);
            // I need to put everything above deleted row one block down
            save_field_state(row, WIDTH);
            for (int upper_row = 0; upper_row < row; upper_row++) {
                for (int col = 1; col < WIDTH - 1; col++) {
                    get_block_at(field, upper_row + 1, col, WIDTH) = get_block_at(oldFieldState, upper_row, col, WIDTH);
                }
            }
            save_field_state(HEIGHT, WIDTH);
            // TODO: change it causes inconsistency
            score += 50;
        }
    }
    return deletedRows;
}

void RenderText(GLuint shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

void ShowGameOver(GLuint shader, GLFWwindow *window) {
    glcall(glClearColor(0.141f, 0.482f, 0.627f, 1.0f));
    glcall(glClear(GL_COLOR_BUFFER_BIT));

    RenderText(shader, "Game Over", 5 * QUAD_SIZE, 10 * QUAD_SIZE, 2.0f, glm::vec3(1.0f, 0.086f, 0.329f));
    std::ostringstream oss;
    oss << "Final Score: " << score;
    RenderText(shader, oss.str(), 3 * QUAD_SIZE, 8 * QUAD_SIZE, 2.0f, glm::vec3(1.0f, 0.086f, 0.329f));

    glfwSwapBuffers(window);

    // Sleep for 1.5 seconds until closing the window
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(2500));
}
