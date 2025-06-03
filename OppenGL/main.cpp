#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>

#include "shader.h"
#include "stb_image.h"
#include "chess_piece.h"
 
const float quadLeft = -0.05f;
const float quadRight = 0.05f;
const float quadTop = 0.05f;
const float quadBottom = -0.05f;

bool isdrag = false; 
bool shouldSnap = false;

float dragOffsetX = 0.0f;
float dragOffsetY = 0.0f;

ChessBoard* globalChessBoard = nullptr;
ChessPiece* draggedPiece = nullptr;
Position draggedPieceOriginalPos(-1, -1);
float draggedPieceX = 0.0f;
float draggedPieceY = 0.0f;

PieceColor currentPlayer = PieceColor::WHITE;
bool boardFlipped = false;
float flipTransition = 0.0f;
bool isFlipping = false;
float flipSpeed = 3.0f;

struct Cell {
    float centerY;
    float centerX;
    float size;
    Position boardPos;
};

Position screenToBoardPosition(float screenX, float screenY) {
    const float boardStart = -0.5f;
    const float cellSize = 1.0f / 8.0f;

    int boardX = (int)((screenX - boardStart) / cellSize);
    int boardY = (int)((screenY - boardStart) / cellSize);

    if (boardFlipped) {
        boardX = 7 - boardX;
        boardY = 7 - boardY;
    }

    // Ограничиваем значения в пределах доски
    boardX = std::max(0, std::min(7, boardX));
    boardY = std::max(0, std::min(7, boardY));

    return Position(boardX, boardY);
}

void boardToRenderCoords(Position boardPos, float& renderX, float& renderY) {
    const float cellSize = 1.0f / 8.0f;

    Position renderPos = boardPos;

    if (boardFlipped) {
        renderPos.x = 7 - boardPos.x;
        renderPos.y = 7 - boardPos.y;
    }

    renderX = -0.5f + cellSize * renderPos.x + cellSize / 2.0f;
    renderY = -0.5f + cellSize * renderPos.y + cellSize / 2.0f;
}

void switchPlayer() {
    currentPlayer = (currentPlayer == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

    if (!isFlipping) {
        isFlipping = true;
    }
}

void updateFlipAnimation(float deltaTime) {
    if (isFlipping) {
        if (boardFlipped) {
            flipTransition -= flipSpeed * deltaTime;
            if (flipTransition <= 0.0f) {
                flipTransition = 0.0f;
                boardFlipped = false;
                isFlipping = false;
            }

        }
        else {
            flipTransition += flipSpeed * deltaTime;
            if (flipTransition >= 1.0f) {
                flipTransition = 1.0f;
                boardFlipped = true;
                isFlipping = false;
            }
        }
    }
}


bool isPointInPiece(float x, float y, ChessPiece* piece) {
    if (piece == nullptr) return false;

    Position piecePos = piece->getPosition();
    float pieceRenderX, pieceRenderY;
    boardToRenderCoords(piecePos, pieceRenderX, pieceRenderY);

    return (x >= pieceRenderX + quadLeft && x <= pieceRenderX + quadRight &&
        y >= pieceRenderY + quadBottom && y <= pieceRenderY + quadTop);
}

ChessPiece* findPieceUnderCursor(float x, float y) {
    if (globalChessBoard == nullptr) return nullptr;

    auto& board = globalChessBoard->getBoard();
    for (int boardY = 0; boardY < 8; boardY++) {
        for (int boardX = 0; boardX < 8; boardX++) {
            ChessPiece* piece = board[boardY][boardX];
            if (piece != nullptr && isPointInPiece(x, y, piece)) {
                return piece;
            }
        }
    }
    return nullptr;
}

Cell* findNearestCell(float x, float y, std::vector<Cell>& cells) {
    Cell* nearest = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    for (auto& cell : cells) {
        float dx = x - cell.centerX;
        float dy = y - cell.centerY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance < minDistance) {
            minDistance = distance;
            nearest = &cell;
        }
    }
    return nearest;
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isdrag && draggedPiece != nullptr) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float x = (2.0f * xpos) / width - 1.0f;
        float y = 1.0f - (2.0f * ypos) / height;

        if (boardFlipped) {
            draggedPieceX = x - dragOffsetX;
            draggedPieceY = y - dragOffsetY;
        }
        else {
            draggedPieceX = x - dragOffsetX;
            draggedPieceY = y - dragOffsetY;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            // Преобразование координат экрана в координаты OpenGL
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            float x = (2.0f * xpos) / width - 1.0f;
            float y = 1.0f - (2.0f * ypos) / height;

            ChessPiece* clickedPiece = findPieceUnderCursor(x, y);

            if (clickedPiece != nullptr)
            {
                isdrag = true;
                shouldSnap = true;
                draggedPiece = clickedPiece;
                draggedPieceOriginalPos = clickedPiece->getPosition();

                float pieceRenderX, pieceRenderY;
                boardToRenderCoords(draggedPieceOriginalPos, pieceRenderX, pieceRenderY);

                draggedPieceX = pieceRenderX; 
                draggedPieceY = pieceRenderY;

                if (boardFlipped) {
                    dragOffsetX = x - pieceRenderX;
                    dragOffsetY = y - pieceRenderY;
                }
                else {
                    dragOffsetX = x - pieceRenderX;
                    dragOffsetY = y - pieceRenderY;
                }
            }
        }
        else if (action == GLFW_RELEASE) {
            if (isdrag && draggedPiece != nullptr) {
                isdrag = false;

                Position targetPos = screenToBoardPosition(draggedPieceX, draggedPieceY);

                if (draggedPiece->getColor() != currentPlayer) {
                    std::cout << "Не ваша очередь! Текущий игрок: "
                        << (currentPlayer == PieceColor::WHITE ? "WHITE" : "BLACK") << std::endl;
                    boardToRenderCoords(draggedPieceOriginalPos, draggedPieceX, draggedPieceY);
                }
                else if (draggedPiece->canMoveTo(targetPos, globalChessBoard->getBoard())) {

                    if (globalChessBoard->movePiece(draggedPieceOriginalPos, targetPos)) {
                        switchPlayer();
                        std::cout << "ход выполнен! Текущий игрок: " << (currentPlayer == PieceColor::WHITE ? "WHITE" : "BLACK") << std::endl;
                    }
                    else {
                        std::cout << "Ошибка при выполнении хода!" << std::endl;
                        boardToRenderCoords(draggedPieceOriginalPos, draggedPieceX, draggedPieceY);
                    }
                }
                else {
                    std::cout << "Недопустимый ход! Возвращаем фигуру на исходную позицию." << std::endl;
                    auto possibleMoves = draggedPiece->getPossibleMoves(globalChessBoard->getBoard());
                    std::cout << "Возможные ходы: ";
                    for (const auto& move : possibleMoves) {
                        std::cout << "(" << move.x << "," << move.y << ") ";
                    }
                    std::cout << std::endl;

                    // Возвращаем фигуру на исходную позицию
                    boardToRenderCoords(draggedPieceOriginalPos, draggedPieceX, draggedPieceY);
                }

                // Сбрасываем состояние перетаскивания
                draggedPiece = nullptr;
                draggedPieceOriginalPos = Position(-1, -1);
            }
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLfloat mixValue = 0.2f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        mixValue += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mixValue -= 0.01f;
}

unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return textureID;

}

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Hello Window!", NULL, NULL); 
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback); 
    glfwSetCursorPosCallback(window, mouse_callback); 

    Shader ourShader("vertex.glsl", "fragment.glsl");
    Shader ourShader_plate("vertex_plate.glsl", "fragment.glsl");
    Shader cellShader("vertex_cell.glsl", "fragment_cell.glsl");

    unsigned int texture1 = loadTexture("texture/brickwall.jpg");
    unsigned int texture2 = loadTexture("texture/brickwall_normal.jpg");

    unsigned int plate_texture = loadTexture("texture/chess_plate.jpeg");

    ChessBoard chessBoard;
    globalChessBoard = &chessBoard;

    std::vector<Cell> cells;
    const int gridSize = 8;
    const float cellSize = 1.0f / gridSize; // От -1 до 1 по каждой оси

    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            Cell cell;
            cell.centerX = -0.5f + cellSize * i + cellSize / 2.0f;
            cell.centerY = -0.5f + cellSize * j + cellSize / 2.0f;
            cell.size = cellSize * 0.8f;
            cells.push_back(cell);
        }
    }

    ChessPiece* white_Rook = ChessPiece::createRook(PieceColor::WHITE, Position(0, 0)); 
    chessBoard.placePiece(white_Rook, Position(0, 0));

    white_Rook->loadTexture("texture/white_rook.png"); 

    ChessPiece* white_knight_left = ChessPiece::createKnight(PieceColor::WHITE, Position(1, 0));
    chessBoard.placePiece(white_knight_left, Position(1, 0)); 

    white_knight_left->loadTexture("texture/white_knight.png"); 

    ChessPiece* white_bishop_left = ChessPiece::createBishop(PieceColor::WHITE, Position(2, 0));
    chessBoard.placePiece(white_bishop_left, Position(2, 0)); 

    white_bishop_left->loadTexture("texture/white_bishop.png"); 

    ChessPiece* white_queen = ChessPiece::createQueen(PieceColor::WHITE, Position(3, 0)); 
    chessBoard.placePiece(white_queen, Position(3, 0)); 

    white_queen->loadTexture("texture/white_queen.png");

    ChessPiece* white_king = ChessPiece::createKing(PieceColor::WHITE, Position(4, 0));
    chessBoard.placePiece(white_king, Position(4, 0));
    
    white_king->loadTexture("texture/white_king.png");

    ChessPiece* white_bishop = ChessPiece::createBishop(PieceColor::WHITE, Position(5, 0));
    chessBoard.placePiece(white_bishop, Position(5, 0));

    white_bishop->loadTexture("texture/white_bishop.png");

    ChessPiece* white_knight = ChessPiece::createKnight(PieceColor::WHITE, Position(6, 0));
    chessBoard.placePiece(white_knight, Position(6, 0));

    white_knight->loadTexture("texture/white_knight.png");

    ChessPiece* superPiece = new ChessPiece(PieceColor::WHITE, Position(7, 0),
        true, true, true, false, true, "Дракон");
    chessBoard.placePiece(superPiece, Position(7, 0));
    superPiece->loadTexture("texture/white_rook.png");

    for (int i = 0; i < 8; i++) {
        ChessPiece* white_pawn = ChessPiece::createPawn(PieceColor::WHITE, Position(i, 1));
        chessBoard.placePiece(white_pawn, Position(i, 1)); 
        white_pawn->loadTexture("texture/white_pawn.png"); 
    }

    ChessPiece* rook_left = ChessPiece::createRook(PieceColor::BLACK, Position(0, 7));
    chessBoard.placePiece(rook_left, Position(0, 7));

    rook_left->loadTexture("texture/rook.png");

    ChessPiece* knight_left = ChessPiece::createKnight(PieceColor::BLACK, Position(1, 7));
    chessBoard.placePiece(knight_left, Position(1, 7));

    knight_left->loadTexture("texture/knight.png");

    ChessPiece* bishop_left = ChessPiece::createBishop(PieceColor::BLACK, Position(2, 7));
    chessBoard.placePiece(bishop_left, Position(2, 7)); 

    bishop_left->loadTexture("texture/bishop.png"); 

    ChessPiece* queen = ChessPiece::createQueen(PieceColor::BLACK, Position(3, 7));
    chessBoard.placePiece(queen, Position(3, 7));

    queen->loadTexture("texture/queen.png");


    ChessPiece* King = ChessPiece::createKing(PieceColor::BLACK, Position(4, 7));
    chessBoard.placePiece(King, Position(4, 7));

    King->loadTexture("texture/king.png");

    ChessPiece* bishop = ChessPiece::createBishop(PieceColor::BLACK, Position(5, 7));
    chessBoard.placePiece(bishop, Position(5, 7));

    bishop->loadTexture("texture/bishop.png");

    ChessPiece* knight = ChessPiece::createKnight(PieceColor::BLACK, Position(6, 7));
    chessBoard.placePiece(knight, Position(6, 7));

    knight->loadTexture("texture/knight.png");

    ChessPiece* rook = ChessPiece::createRook(PieceColor::BLACK, Position(7, 7));
    chessBoard.placePiece(rook, Position(7, 7));

    rook->loadTexture("texture/rook.png");

    for (int i = 0; i < 8; i++) {
        ChessPiece* pawn = ChessPiece::createPawn(PieceColor::BLACK, Position(i, 6));
        chessBoard.placePiece(pawn, Position(i, 6)); 
        pawn->loadTexture("texture/pawn.png");
    }
     
    GLfloat vertices_plate[] = { 
        // Positions          // Colors           // Texture Coords
         0.6f,  0.6f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,  // Top Right
         0.6f, -0.6f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,  // Bottom Right
        -0.6f, -0.6f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,  // Bottom Left
        -0.6f,  0.6f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f   // Top Left
    };

    GLuint indices_plate[] = {
        0,1,3,
        1,2,3
    };

    GLuint VBO2, VAO2, EBO2;
    glGenBuffers(1, &VBO2);
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &EBO2);

    glBindVertexArray(VAO2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_plate), vertices_plate, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_plate), indices_plate, GL_STATIC_DRAW);




    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);



    //glVertexAttribPointer(1, 2, GL_FLOAT, GLFW_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    //glEnableVertexAttribArray(1);

    GLuint cellVAO, cellVBO, cellEBO; 
    glGenVertexArrays(1, &cellVAO); 
    glGenBuffers(1, &cellVBO); 
    glGenBuffers(1, &cellEBO); 

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        updateFlipAnimation(deltaTime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        processInput(window);

        ourShader_plate.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, plate_texture);
        glUniform1i(glGetUniformLocation(ourShader_plate.Program, "ourTexture1"), 0);
        glBindVertexArray(VAO2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        cellShader.Use();
        glUniform1f(glGetUniformLocation(cellShader.Program, "mixValue"), 0.8f);
        for (const auto& cell : cells) {
            GLfloat cellVertices[] = {
                cell.centerX - 0.07f + cell.size / 2, cell.centerY + cell.size / 2, 0.0f,
                cell.centerX - 0.07f + cell.size / 2, cell.centerY - cell.size / 2, 0.0f,
                cell.centerX + cell.size / 2, cell.centerY - cell.size / 2, 0.0f,
                cell.centerX + cell.size / 2, cell.centerY + cell.size / 2, 0.0f
            };
            glBindVertexArray(cellVAO);
            glBindBuffer(GL_ARRAY_BUFFER, cellVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cellVertices), cellVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(0);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
        }

        ourShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);
        glUniform1f(glGetUniformLocation(ourShader.Program, "mixValue"), mixValue);

        glUniform1f(glGetUniformLocation(ourShader.Program, "flipTransition"), flipTransition);
        glUniform1i(glGetUniformLocation(ourShader.Program, "boardFlipped"), boardFlipped ? 1 : 0);

        auto& board = chessBoard.getBoard();
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                ChessPiece* piece = board[y][x];
                if (piece != nullptr && piece != draggedPiece) {
                    Position renderPos(x, y);
                    if (boardFlipped) {
                        renderPos.x = 7 - x;
                        renderPos.y = 7 - y;
                    }

                    float renderX, renderY;
                    const float cellSize = 1.0f / 8.0f;
                    renderX = -0.5f + cellSize * renderPos.x + cellSize / 2.0f;
                    renderY = -0.5f + cellSize * renderPos.y + cellSize / 2.0f;

                    piece->setRenderPosition(renderX, renderY);
                    piece->render(ourShader);
                }
            }
        }

        if (draggedPiece != nullptr && isdrag) {

            float finalX = draggedPieceX;
            float finalY = draggedPieceY;

            glUniform1f(glGetUniformLocation(ourShader.Program, "x_ran"), finalX);
            glUniform1f(glGetUniformLocation(ourShader.Program, "y_ran"), finalY);

            glBindVertexArray(draggedPiece->VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &EBO2);
    glfwTerminate();
    return 0;
}