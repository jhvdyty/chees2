# chees2
chees2 is an interactive OpenGL-based chessboard simulation built using modern C++ and GLFW. It features drag-and-drop chess piece movement, real-time piece interaction, and a smooth board flipping animation.

# Features
Rendered chessboard using OpenGL 3.3 Core Profile

Full 8×8 grid with pieces positioned in standard chess order

Drag and drop interaction with mouse

Turn-based movement (white vs black)

Basic movement validation (no full chess rules yet)

Board flipping animation between player turns

Custom textures for all pieces

Uses GLSL vertex and fragment shaders

Chess piece logic abstracted into classes

# Project Structure
graphql
Копировать
Редактировать
chees2/
│
├── OppenGL/
│   ├── main.cpp              # Entry point of the application
│   ├── chess_piece.h         # Piece classes and logic
│   ├── shader.h              # Shader abstraction for OpenGL
│   ├── stb_image.h           # Image loading (for textures)
│   ├── test.cpp              # Optional testing or debugging code
│   ├── vertex.glsl           # Vertex shader for general pieces
│   ├── fragment.glsl         # Fragment shader for general pieces
│   ├── vertex_cell.glsl      # Vertex shader for board cells
│   ├── fragment_cell.glsl    # Fragment shader for board cells
│   ├── vertex_plate.glsl     # Vertex shader for the board plate
│   ├── fragment_plate.glsl   # Fragment shader for the board plate
│
├── texture/                  # Textures used for pieces and board
│   ├── white_pawn.png
│   ├── black_pawn.png
│   ├── white_king.png
│   ├── black_king.png
│   ├── ...
│
├── OppenGL.sln               # Visual Studio solution file
├── OppenGL.vcxproj           # Visual Studio project file
├── OppenGL.vcxproj.filters   # File filters for VS
├── .gitignore
├── .gitattributes

# Requirements
C++17 or newer

OpenGL 3.3+

GLFW

GLAD

stb_image.h

These dependencies must be correctly linked into your Visual Studio project.

# How to Build
Clone the repository

Open OppenGL.sln in Visual Studio

Ensure all .cpp/.h/.glsl and texture/ files are included in the project

Build the solution

Run the compiled application

You should see a chessboard rendered with interactive pieces. Click and drag to move them. The board flips automatically after each valid move.

# Controls
Key	Action
ESC	Exit the application
Mouse drag	Move a piece
W / S	Increase / decrease mix value (unused currently)

# Notes
This project is a visual and interactive simulation. It does not implement full chess rules (e.g., checkmate, en passant, castling).

The game enforces alternating turns between white and black.

Board flipping provides a better player-versus-player experience on a shared screen.

# Screenshot
![image](https://github.com/user-attachments/assets/5317edac-1d53-4e65-9314-9f6fa4aa022a)

![image](https://github.com/user-attachments/assets/e611e0c7-5099-482f-807a-57aec933a05c)


# License
This project is for educational and demonstration purposes. You are free to modify and reuse the code.
