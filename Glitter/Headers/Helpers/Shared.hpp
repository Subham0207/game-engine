#pragma once
#include <string>
#include <filesystem>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
namespace fs = std::filesystem;
namespace Shared {
    unsigned int sendTextureToGPU(unsigned char* data, int mWidth, int mheight, int nrComponents);

    unsigned int TextureFromFile(const char* path, std::string filename, bool save = true);

    unsigned int generateTexture(unsigned char* pixel);

    unsigned int generateMetallicTexture();
    unsigned int generateNonMetallicTexture();
    unsigned int generateWhiteAOTexture();
    unsigned int generateFlatNormalTexture();

    void readAnimation(std::string filename);

    bool endsWith(const std::string& value, const std::string& ending);

    void WriteTextFile(const fs::path& p, const std::string& s);

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    int initAWindow(GLFWwindow* window);

    void initImguiBackend(GLFWwindow* window);

    void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
}