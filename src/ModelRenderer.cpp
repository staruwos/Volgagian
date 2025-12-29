#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// This define is needed because we downloaded json.hpp to src/ 
// but tinygltf expects it in include path. We simplify by including it here.
#include "json.hpp" 
#include "tiny_gltf.h"

#include "ModelRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

ModelRenderer::ModelRenderer(const std::string& path) 
{

    position = glm::vec3(0.0f);
    rotation = glm::vec3(0.0f);
    scale    = glm::vec3(1.0f);

    shader = new Shader("data/shaders/model.vert", "data/shaders/model.frag");

    // Load GLB
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty()) std::cout << "GLTF Warn: " << warn << std::endl;
    if (!err.empty()) std::cerr << "GLTF Err: " << err << std::endl;
    if (!ret) {
        std::cerr << "Failed to parse GLTF: " << path << std::endl;
        return;
    }

    // Process Scene Nodes (Simplified: Just grab all meshes)
    for (const auto& mesh : model.meshes) {
        processMesh(model, mesh);
    }
}

void ModelRenderer::processMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) 
{
    for (const auto& primitive : mesh.primitives) {
        MeshData glMesh;
        glMesh.textureID = 0;

        glGenVertexArrays(1, &glMesh.VAO);
        glGenBuffers(1, &glMesh.VBO);
        glGenBuffers(1, &glMesh.EBO);

        glBindVertexArray(glMesh.VAO);

        // --- Indices ---
        const tinygltf::Accessor& idxAcc = model.accessors[primitive.indices];
        glMesh.indexType = idxAcc.componentType;
        const tinygltf::BufferView& idxView = model.bufferViews[idxAcc.bufferView];
        const tinygltf::Buffer& idxBuffer = model.buffers[idxView.buffer];

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxView.byteLength, 
                     &idxBuffer.data[idxAcc.byteOffset + idxView.byteOffset], GL_STATIC_DRAW);
        glMesh.indexCount = idxAcc.count;
        

        // --- Vertices (POSITION) ---
        if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
            const int posAccIdx = primitive.attributes.at("POSITION");
            const tinygltf::Accessor& posAcc = model.accessors[posAccIdx];
            const tinygltf::BufferView& posView = model.bufferViews[posAcc.bufferView];
            const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];

            glBindBuffer(GL_ARRAY_BUFFER, glMesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, posView.byteLength, 
                         &posBuffer.data[posAcc.byteOffset + posView.byteOffset], GL_STATIC_DRAW);

            // layout (location = 0) = vec3 aPos
            glVertexAttribPointer(0, 3, posAcc.componentType, 
                                  posAcc.normalized ? GL_TRUE : GL_FALSE, 
                                  posView.byteStride ? posView.byteStride : 3 * sizeof(float), 
                                  (void*)0);
            glEnableVertexAttribArray(0);
        }

        // TEXCOORD_0 (Location 2)
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const int uvAccIdx = primitive.attributes.at("TEXCOORD_0");
            const tinygltf::Accessor& uvAcc = model.accessors[uvAccIdx];
            const tinygltf::BufferView& uvView = model.bufferViews[uvAcc.bufferView];
            const tinygltf::Buffer& uvBuffer = model.buffers[uvView.buffer];
            
            GLuint uvVBO;
            glGenBuffers(1, &uvVBO);
            glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
            glBufferData(GL_ARRAY_BUFFER, uvView.byteLength, 
                         &uvBuffer.data[uvAcc.byteOffset + uvView.byteOffset], GL_STATIC_DRAW);

            glVertexAttribPointer(2, 2, uvAcc.componentType, 
                                  uvAcc.normalized ? GL_TRUE : GL_FALSE, 
                                  uvView.byteStride ? uvView.byteStride : 2 * sizeof(float), 
                                  (void*)0);
            glEnableVertexAttribArray(2);
        }

        // Load Material/Texture
        if (primitive.material >= 0) {
            const tinygltf::Material& mat = model.materials[primitive.material];
            // Check for PBR Base Color Texture
            int texIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
            if (texIndex >= 0) {
                glMesh.textureID = loadTexture(model, texIndex);
            }
        }

        glBindVertexArray(0);


        meshes.push_back(glMesh);
    }
}

GLuint ModelRenderer::loadTexture(const tinygltf::Model& model, int textureIndex) {
    if (textureIndex < 0 || textureIndex >= model.textures.size()) return 0;

    const tinygltf::Texture& tex = model.textures[textureIndex];
    if (tex.source < 0 || tex.source >= model.images.size()) return 0;

    const tinygltf::Image& image = model.images[tex.source];

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Setup wrap/filters (Basic defaults)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload Data
    // tinygltf usually loads images as RGBA unsigned bytes
    GLenum format = GL_RGBA;
    if (image.component == 3) format = GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, 
                 format, GL_UNSIGNED_BYTE, &image.image[0]);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

void ModelRenderer::draw(const glm::mat4& view, const glm::mat4& projection) {
    if (meshes.empty()) return;

    shader->use();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // --- Build Model Matrix from State ---
    glm::mat4 model = glm::mat4(1.0f);

    // Translate
    model = glm::translate(model, position);

    // Rotate (Order: Y -> X -> Z is usually best for RPGs)
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

    // Scale
    model = glm::scale(model, scale);

    // Send Matrices
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model);
    shader->setVec3("uColor", 1.0f, 0.5f, 0.2f); 

    shader->setInt("texture_diffuse", 0);

    for (const auto& mesh : meshes) {
        // Activate Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        
        // Bind the specific texture for this mesh part
        if (mesh.textureID != 0) {
            glBindTexture(GL_TEXTURE_2D, mesh.textureID);
        } else {
            // Unbind if no texture, or bind a white pixel texture here 
            glBindTexture(GL_TEXTURE_2D, 0); 
        }

        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, mesh.indexType, 0);
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}