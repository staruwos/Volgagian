#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

#include "Math.h"
#include "TileRenderer.h"
#include "ModelRenderer.h"

// Global mouse pos
double mouseX, mouseY;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
} 
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure for OpenGL 3.3 Core Profile (Max Compatibility baseline)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // Apple compatibility
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create Window (800x600 resolution)
    GLFWwindow* window = glfwCreateWindow(800, 600, "Volgagian", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Mouse callback
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Check if the Context was actually created properly by GLFW
    const GLubyte* g_renderer = glGetString(GL_RENDERER);
    const GLubyte* g_version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << (g_renderer ? (const char*)g_renderer : "NULL") << std::endl;
    std::cout << "OpenGL Version: " << (g_version ? (const char*)g_version : "NULL") << std::endl;

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    // IGNORE the "Unknown Error" (Error Code 4) on Wayland
    if (err != GLEW_OK && err != 4) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // Initialize Tile Renderer
    TileRenderer* renderer = new TileRenderer();
    ModelRenderer* soldier = new ModelRenderer("data/models/Soldier.glb");
    glm::vec3 correction = glm::vec3(-90.0f, 0.0f, 0.0f); 
    
    soldier->setScale(glm::vec3(0.01f)); 
    soldier->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    soldier->setRotation(correction);

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        
        // Calculate Mouse Tile
        glm::vec2 worldPos((float)mouseX - 400.0f, (float)mouseY - 100.0f);
        glm::ivec2 hoveredTile = VMath::screenToTile(worldPos);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        // Draw a 10x10 Grid
        for (int y = 0; y < 10; y++) {
            for (int x = 0; x < 10; x++) {
                glm::ivec2 tilePos(x, y);
                
                bool isHovered = (tilePos.x == hoveredTile.x && tilePos.y == hoveredTile.y);
                
                renderer->drawTile(tilePos, 800, 600, isHovered);
            }
        }

        float aspect = 800.0f / 600.0f;
        float viewSize = 5.0f;
        
        // Projection
        glm::mat4 proj = glm::ortho(-viewSize * aspect, viewSize * aspect, -viewSize, viewSize, -100.0f, 100.0f);
        
        // View
        glm::mat4 view = glm::lookAt(
            glm::vec3(20.0f, 20.0f, 20.0f), // Camera Pos
            glm::vec3(0.0f, 0.0f, 0.0f),    // Target
            glm::vec3(0.0f, 1.0f, 0.0f)     // Up
        );

        // Update & Draw Soldier
        // Manually update rotation to keep it spinning
        float time = (float)glfwGetTime();
        //soldier->setRotation(glm::vec3(time * 90.0f, 90.0f, 90.0f)); // Spin 50 degrees per second on Y

        soldier->draw(view, proj);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
