#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>

#include "shader.h"
#include "stb_image.h"
 
const float quadLeft = -0.15f;
const float quadRight = 0.15f;
const float quadTop = 0.15f;
const float quadBottom = -0.15f;

double random_number_y;
double random_number_x;

float x_ran = 0.0f;
float y_ran = 0.0f;

bool isdrag = false; 
bool shouldSnap = false;

float dragOffsetX = 0.0f;
float dragOffsetY = 0.0f;

struct Cell {
    float centerY;
    float centerX;
    float size;
};

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
    if (isdrag) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float x = (2.0f * xpos) / width - 1.0f;
        float y = 1.0f - (2.0f * ypos) / height;

        x_ran = x - dragOffsetX;
        y_ran = y - dragOffsetY;
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

            if (x >= quadLeft + x_ran && x <= quadRight + x_ran && y >= quadBottom + y_ran && y <= quadTop + y_ran)
            {
                isdrag = true;

                shouldSnap = true;

                dragOffsetX = x - x_ran;
                dragOffsetY = y - y_ran;

            }
        }
        else {
            isdrag = false;
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Window!", NULL, NULL);
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

    Shader ourShader("vertex.glsl", "fragment.glsl");
    Shader ourShader_plate("vertex_plate.glsl", "fragment.glsl");
    Shader cellShader("vertex_cell.glsl", "fragment_cell.glsl");

    unsigned int texture1 = loadTexture("texture/brickwall.jpg");
    unsigned int texture2 = loadTexture("texture/brickwall_normal.jpg");

    unsigned int plate_texture = loadTexture("texture/chess_plate.jpeg");


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

    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords
         0.15f,  0.15f, 0.0f,   0.5f, 0.0f, 0.0f,   0.5f, 0.5f, // Top Righ // Top Right
         0.15f, -0.15f, 0.0f,   0.0f, 0.5f, 0.0f,   0.5f, 0.0f, // Bottom Right
        -0.15f, -0.15f, 0.0f,   0.0f, 0.0f, 0.5f,   0.0f, 0.0f, // Bottom Left
        -0.15f,  0.15f, 0.0f,   0.5f, 0.5f, 0.0f,   0.0f, 0.5f  // Top Left 
    };

    GLuint indices[] = {
        0,1,3,
        1,2,3
    };

    GLuint VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);




    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
     
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

 

    while (!glfwWindowShouldClose(window)) {
        // ... Обработка событий (если необходимо) ...

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // ... Команды отрисовки ...

        //GLfloat timeValue = glfwGetTime();
        //GLfloat greenValue = (sin(timeValue) / 2) + 0.5;
        //GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        
        processInput(window);

        glfwSetMouseButtonCallback(window, mouse_button_callback); 
        glfwSetCursorPosCallback(window, mouse_callback);

        if (!isdrag && shouldSnap) {
            Cell* nearestCell = findNearestCell(x_ran, y_ran, cells);
            if (nearestCell) {
                x_ran = nearestCell->centerX;
                y_ran = nearestCell->centerY;
            }
            shouldSnap = false;
        }


        // Перед отрисовкой пластины:
        ourShader_plate.Use(); 
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, plate_texture); 
        glUniform1i(glGetUniformLocation(ourShader_plate.Program, "ourTexture1"), 0);

        glBindVertexArray(VAO2); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Сначала пластина 

        cellShader.Use();
        glUniform1f(glGetUniformLocation(cellShader.Program, "x_ran"), 0.0f);
        glUniform1f(glGetUniformLocation(cellShader.Program, "y_ran"), 0.0f);
        glUniform1f(glGetUniformLocation(cellShader.Program, "mixValue"), 0.8f);

        for (const auto& cell : cells) {
            // Создаем вершины для каждой ячейки
            GLfloat cellVertices[] = {
                cell.centerX -0.07 + cell.size / 2, cell.centerY + cell.size / 2, 0.0f, // TR
                cell.centerX -0.07 + cell.size / 2, cell.centerY - cell.size / 2, 0.0f, // BR
                cell.centerX + cell.size / 2, cell.centerY - cell.size / 2, 0.0f, // BL
                cell.centerX + cell.size / 2, cell.centerY + cell.size / 2, 0.0f  // TL
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

        glUniform1f(glGetUniformLocation(ourShader.Program, "x_ran"), x_ran);
        glUniform1f(glGetUniformLocation(ourShader.Program, "y_ran"), y_ran);

        glBindVertexArray(VAO); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Затем квадрат 
        //glDrawArrays(GL_TRIANGLES, 6, 0);
        //vertexColorLocation = glGetUniformLocation(secondShaderProgram, "ourColor");
        //glUniform4f(vertexColorLocation, 0.5f, greenValue, 0.7f, 1.0f);

        glBindVertexArray(0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glDeleteVertexArrays(1, &VAO2); 
    glDeleteBuffers(1, &VBO2); 
    glDeleteBuffers(1, &EBO2); 

    glfwTerminate();
    return 0;
}
