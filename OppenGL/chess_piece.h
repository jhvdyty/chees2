#ifndef CHESS_H
#define CHESS_H
#include <glad/glad.h> 

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#include <GLFW/glfw3.h>
#include "shader.h"
#include "stb_image.h"

enum class PieceColor {
    WHITE,
    BLACK
};

struct Position {
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

class ChessPiece
{
protected:
    unsigned int texture;
    PieceColor color;
    Position position;
    float renderX, renderY;

    // способности
    bool canMoveAsRook;
    bool canMoveAsBishop;
    bool canMoveAsKnight;
    bool canMoveAsPawn;
    bool canMoveAsKing;
    
    // свойства
    bool hasMoved;
    std::string pieceName;

public:
    unsigned int VAO, VBO, EBO;

    ChessPiece(PieceColor pieceColor, Position pos,
        bool rook = false, bool bishop = false, bool knight = false,
        bool pawn = false, bool king = false,
        const std::string& name = "Custom Piece")
        : color(pieceColor), position(pos), canMoveAsRook(rook),
        canMoveAsBishop(bishop), canMoveAsKnight(knight),
        canMoveAsPawn(pawn), canMoveAsKing(king),
        hasMoved(false), pieceName(name), texture(0) {

        setupMesh();
        convertToRenderCoords();
    }

    virtual ~ChessPiece() {
        cleanup();
    }

    virtual std::vector<Position> getPossibleMoves(const std::vector<std::vector<ChessPiece*>>& board) const {
        std::vector<Position> moves;

        if (canMoveAsRook) {
            addRookMoves(moves, board);
        }
        if (canMoveAsBishop) {
            addBishopMoves(moves, board);
        }
        if (canMoveAsKnight) {
            addKnightMoves(moves, board);
        }
        if (canMoveAsPawn) {
            addPawnMoves(moves, board);
        }
        if (canMoveAsKing) {
            addKingMoves(moves, board);
        }

        return moves;
    }

    virtual bool canMoveTo(const Position& target, const std::vector<std::vector<ChessPiece*>>& board) const {
        auto possibleMoves = getPossibleMoves(board);
        return std::find(possibleMoves.begin(), possibleMoves.end(), target) != possibleMoves.end();
    }

    virtual bool moveTo(const Position& target, std::vector<std::vector<ChessPiece*>>& board) {
        if (canMoveTo(target, board)) {
            // Удаляем фигуру с текущей позиции
            board[position.y][position.x] = nullptr;

            // Удаляем захваченную фигуру (если есть)
            if (board[target.y][target.x] != nullptr) {
                delete board[target.y][target.x];
            }

            // Устанавливаем фигуру на новую позицию
            board[target.y][target.x] = this;
            position = target;
            hasMoved = true;
            convertToRenderCoords();
            return true;
        }
        return false;
    }

    // рендеринг
    virtual void render(Shader& shader) {
        shader.Use();
        glUniform1f(glGetUniformLocation(shader.Program, "x_ran"), renderX);
        glUniform1f(glGetUniformLocation(shader.Program, "y_ran"), renderY);

        if (texture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(glGetUniformLocation(shader.Program, "ourTexture1"), 0);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void loadTexture(const std::string& texturePath) {
        texture = loadTextureFromFile(texturePath.c_str());
    }

    // Геттеры
    PieceColor getColor() const { return color; }
    Position getPosition() const { return position; }
    std::string getName() const { return pieceName; }
    bool getHasMoved() const { return hasMoved; }

    bool hasRookMovement() const { return canMoveAsRook; }
    bool hasBishopMovement() const { return canMoveAsBishop; }
    bool hasKnightMovement() const { return canMoveAsKnight; }
    bool hasPawnMovement() const { return canMoveAsPawn; }
    bool hasKingMovement() const { return canMoveAsKing; }

    // рендеринг
    void setRenderPosition(float x, float y) {
        renderX = x;
        renderY = y;
    }

    static ChessPiece* createRook(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, true, false, false, false, false, "Rook");
    }

    static ChessPiece* createBishop(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, false, true, false, false, false, "Bishop");
    }

    static ChessPiece* createKnight(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, false, false, true, false, false, "Knight");
    }

    static ChessPiece* createPawn(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, false, false, false, true, false, "Pawn");
    }

    static ChessPiece* createKing(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, false, false, false, false, true, "King");
    }

    static ChessPiece* createQueen(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, true, true, false, false, false, "Queen");
    }

    // Создание гибридных фигур
    static ChessPiece* createKnightRook(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, true, false, true, false, false, "Knight-Rook");
    }

    static ChessPiece* createPawnKnight(PieceColor color, Position pos) {
        return new ChessPiece(color, pos, false, false, true, true, false, "Pawn-Knight");
    }

private:
    void setupMesh() {
        GLfloat vertices[] = {
            // Positions          // Colors           // Texture Coords
             0.15f,  0.15f, 0.0f,   0.5f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
             0.15f, -0.15f, 0.0f,   0.0f, 0.5f, 0.0f,   1.0f, 0.0f, // Bottom Right
            -0.15f, -0.15f, 0.0f,   0.0f, 0.0f, 0.5f,   0.0f, 0.0f, // Bottom Left
            -0.15f,  0.15f, 0.0f,   0.5f, 0.5f, 0.0f,   0.0f, 1.0f  // Top Left 
        };

        GLuint indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);


        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void convertToRenderCoords() {
        // позиции доски 0-7 в координаты OpenGL -0.5 до 0.5
        const float cellSize = 1.0f / 8.0f;
        renderX = -0.5f + cellSize * position.x + cellSize / 2.0f;
        renderY = -0.5f + cellSize * position.y + cellSize / 2.0f;
    }

    unsigned int loadTextureFromFile(const char* path) {
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
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture: " << path << std::endl;
        }
        stbi_image_free(data);
        return textureID;
    }

    void cleanup() {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }
    }

    bool isValidPosition(const Position& pos) const {
        return pos.x >= 0 && pos.x < 8 && pos.y >= 0 && pos.y < 8;
    }

    void addRookMoves(std::vector<Position>& moves, const std::vector<std::vector<ChessPiece*>>& board) const {
        int directions[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

        for (int i = 0; i < 4; i++) {
            int dx = directions[i][0];
            int dy = directions[i][1];

            for (int step = 1; step < 8; step++) {
                Position newPos(position.x + dx * step, position.y + dy * step);

                if (!isValidPosition(newPos)) break;

                ChessPiece* targetPiece = board[newPos.y][newPos.x];
                if (targetPiece == nullptr) {
                    moves.push_back(newPos);
                }
                else {
                    if (targetPiece->getColor() != color) {
                        moves.push_back(newPos); 
                    }
                    break; 
                }
            }
        }
    }

    void addBishopMoves(std::vector<Position>& moves, const std::vector<std::vector<ChessPiece*>>& board) const {
        int directions[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };

        for (int i = 0; i < 4; i++) {
            int dx = directions[i][0];
            int dy = directions[i][1];

            for (int step = 1; step < 8; step++) {
                Position newPos(position.x + dx * step, position.y + dy * step);

                if (!isValidPosition(newPos)) break;

                ChessPiece* targetPiece = board[newPos.y][newPos.x];
                if (targetPiece == nullptr) {
                    moves.push_back(newPos);
                }
                else {
                    if (targetPiece->getColor() != color) {
                        moves.push_back(newPos);
                    }
                    break;
                }
            }
        }
    }

    void addKnightMoves(std::vector<Position>& moves, const std::vector<std::vector<ChessPiece*>>& board) const {
        int knightMoves[8][2] = {
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
            {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
        };

        for (int i = 0; i < 8; i++) {
            Position newPos(position.x + knightMoves[i][0], position.y + knightMoves[i][1]);

            if (isValidPosition(newPos)) {
                ChessPiece* targetPiece = board[newPos.y][newPos.x];
                if (targetPiece == nullptr || targetPiece->getColor() != color) {
                    moves.push_back(newPos);
                }
            }
        }
    }

    void addPawnMoves(std::vector<Position>& moves, const std::vector<std::vector<ChessPiece*>>& board) const {
        int direction = (color == PieceColor::WHITE) ? 1 : -1;

        // вперед
        Position frontPos(position.x, position.y + direction);
        if (isValidPosition(frontPos) && board[frontPos.y][frontPos.x] == nullptr) {
            moves.push_back(frontPos);

            // двойной ход с начала
            if (!hasMoved) {
                Position doubleFrontPos(position.x, position.y + 2 * direction);
                if (isValidPosition(doubleFrontPos) && board[doubleFrontPos.y][doubleFrontPos.x] == nullptr) {
                    moves.push_back(doubleFrontPos);
                }
            }
        }

        // по диагонали
        for (int dx = -1; dx <= 1; dx += 2) {
            Position capturePos(position.x + dx, position.y + direction);
            if (isValidPosition(capturePos)) {
                ChessPiece* targetPiece = board[capturePos.y][capturePos.x];
                if (targetPiece != nullptr && targetPiece->getColor() != color) {
                    moves.push_back(capturePos);
                }
            }
        }
    }

    void addKingMoves(std::vector<Position>& moves, const std::vector<std::vector<ChessPiece*>>& board) const {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;

                Position newPos(position.x + dx, position.y + dy);
                if (isValidPosition(newPos)) {
                    ChessPiece* targetPiece = board[newPos.y][newPos.x];
                    if (targetPiece == nullptr || targetPiece->getColor() != color) {
                        moves.push_back(newPos);
                    }
                }
            }
        }
    }
};

class ChessBoard {
private:
    std::vector<std::vector<ChessPiece*>> board;

public:
    ChessBoard() {
        board.resize(8, std::vector<ChessPiece*>(8, nullptr));
    }

    ~ChessBoard() {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (board[y][x] != nullptr) {
                    delete board[y][x];
                }
            }
        }
    }

    void placePiece(ChessPiece* piece, Position pos) {
        if (isValidPosition(pos)) {
            if (board[pos.y][pos.x] != nullptr) {
                delete board[pos.y][pos.x];
            }
            board[pos.y][pos.x] = piece;
        }
    }

    ChessPiece* getPiece(Position pos) {
        if (isValidPosition(pos)) {
            return board[pos.y][pos.x];
        }
        return nullptr;
    }

    bool movePiece(Position from, Position to) {
        if (isValidPosition(from) && isValidPosition(to)) {
            ChessPiece* piece = board[from.y][from.x];
            if (piece != nullptr) {
                return piece->moveTo(to, board);
            }
        }
        return false;
    }


    void renderAllPieces(Shader& shader) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (board[y][x] != nullptr) {
                    board[y][x]->render(shader);
                }
            }
        }
    }

    std::vector<std::vector<ChessPiece*>>& getBoard() {
        return board;
    }


private:
    bool isValidPosition(const Position& pos) {
        return pos.x >= 0 && pos.x < 8 && pos.y >= 0 && pos.y < 8;
    }
};


#endif // CHESS_H

