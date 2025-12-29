#pragma once
#include <GL/glew.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>

class Shader {
public:
    GLuint ID;

    // Constructor now takes File Paths, not Source Code
    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode = readFile(vertexPath);
        std::string fragmentCode = readFile(fragmentPath);

        // If reading failed, do not attempt to compile
        if (vertexCode.empty() || fragmentCode.empty()) return;

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        GLuint vertex = compile(GL_VERTEX_SHADER, vShaderCode);
        GLuint fragment = compile(GL_FRAGMENT_SHADER, fShaderCode);
        
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkLinkErrors(ID);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() { glUseProgram(ID); }
    
    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    
    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

private:
    // Reads a file into a std::string
    std::string readFile(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "| ERROR | Shader File Not Found: " << path << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    GLuint compile(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        checkCompileErrors(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
        return shader;
    }

    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        char infoLog[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "| ERROR | Shader Compile: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }

    void checkLinkErrors(GLuint program) {
        GLint success;
        char infoLog[1024];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 1024, NULL, infoLog);
            std::cerr << "| ERROR | Shader Linking\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
};