#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Shader.h"

// Forward declare to avoid including the massive tiny_gltf header here
namespace tinygltf { class Model; class Mesh; }

struct MeshData {
    GLuint VAO, VBO, EBO;
    int indexCount;
    GLenum indexType;
    GLuint textureID;
};

class ModelRenderer {
public:
    Shader* shader;
    std::vector<MeshData> meshes;

    // --- Transform State ---
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in degrees
    glm::vec3 scale;

    ModelRenderer(const std::string& path);
    
    // Setters for convenience
    void setPosition(glm::vec3 pos) { position = pos; }
    void setRotation(glm::vec3 rot) { rotation = rot; }
    void setScale(glm::vec3 s)      { scale = s; }

    void draw(const glm::mat4& view, const glm::mat4& projection);

private:
    void processMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh);

    GLuint loadTexture(const tinygltf::Model& model, int textureIndex);
};