#pragma once
#include <string>
#include <filesystem>

#include <imgui.h>
#include <imnodes.h>

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
    unsigned int generateDefaultRoughnessTexture();
    unsigned int generateFlatNormalTexture();

    void readAnimation(std::string filename);

    bool endsWith(const std::string& value, const std::string& ending);

    void WriteTextFile(const fs::path& p, const std::string& s);

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    GLFWwindow* initAWindow(
        bool& isVsyncOn,
        bool& isToolWindow,
        std::string title,
        bool& isMouseDisabled);

    void initImguiBackend(GLFWwindow* window);

    // --- Multi-window ImGui helpers ---
    // These allow each GLFW window to have its own ImGui/ImNodes contexts and backend state.
    // Use these when creating tool windows to avoid leaking Win32/GLFW backend state.
    ImGuiContext* createImguiContext();
    ImNodesContext* createImNodesContext();

    // Initializes imgui_impl_glfw + imgui_impl_opengl3 for the *current* ImGui context,
    // installing callbacks on the provided GLFW window.
    void initImguiBackendForWindow(GLFWwindow* window);

    // Shuts down imgui_impl_glfw + imgui_impl_opengl3 for the *current* ImGui context.
    // Safe to call only if initImguiBackendForWindow() was called for that same context.
    void shutdownImguiBackendForWindow();

    void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

    fs::path metaFileToActualPath(const fs::path& path);

    void CopyFileToProjectDirectory(std::string& filelocation);

    GLFWwindow* InitBackEndsWithWindow(
        bool& isVsyncOn,
        bool& isToolWindow,
        std::string title,
        bool& isMouseDisabled);

    void initGpuLogger();

    std::string uuid();
}
