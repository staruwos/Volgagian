#pragma once
#include <GL/glew.h>
#include "Shader.h"
#include "Math.h"

class TileRenderer {
private:
    GLuint VAO, VBO;
    Shader* shader;

    // A simple quad centered at 0,0
    // We draw a Diamond shape directly to visualize the isometric tile
    // Top(0, 0.5), Right(1, 0), Bottom(0, -0.5), Left(-1, 0) -> Scaled by logic
    float vertices[12] = {
         0.0f,  0.5f,  // Top
         1.0f,  0.0f,  // Right
         0.0f, -0.5f,  // Bottom
        
         0.0f, -0.5f,  // Bottom
        -1.0f,  0.0f,  // Left
         0.0f,  0.5f   // Top
    };

public:
    TileRenderer() {
        shader = new Shader("data/shaders/tile.vert", "data/shaders/tile.frag");

        // Setup Buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0);
    }

    void drawTile(glm::ivec2 gridPos, float screenWidth, float screenHeight, bool isHovered) {
        shader->use();
        
        shader->setVec2("uResolution", screenWidth, screenHeight);
        shader->setVec2("uSize", VMath::TILE_WIDTH, VMath::TILE_HEIGHT);

        glm::vec2 screenPos = VMath::tileToScreen(gridPos);
        screenPos.x += 400.0f; 
        screenPos.y += 100.0f;

        shader->setVec2("uOffset", screenPos.x, screenPos.y);

        if (isHovered) {
            shader->setVec3("uColor", 0.0f, 1.0f, 0.0f); 
        } else {
            bool dark = (gridPos.x + gridPos.y) % 2 == 0;
            if (dark) shader->setVec3("uColor", 0.4f, 0.4f, 0.4f);
            else      shader->setVec3("uColor", 0.5f, 0.5f, 0.5f);
        }

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
};