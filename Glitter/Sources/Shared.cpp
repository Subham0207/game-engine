#include "Helpers/Shared.hpp"
#include <iostream>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glad/glad.h>
#include <3DModel/Animation/Animation.hpp>
#include <EngineState.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include "Profiler.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"

namespace Shared{
        unsigned int sendTextureToGPU(unsigned char* data, int mWidth, int mheight, int nrComponents){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mheight, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            // std::cout << "Opened and loaded " << path << std::endl;
        }
        else
        {
                std::cerr << "Failed to load texture: "
                << ", Reason: " << stbi_failure_reason() << std::endl;
            stbi_image_free(data);
        }
        return textureID;
    }

    unsigned int TextureFromFile(const char* path, std::string filename, bool save)
    {
        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if(save)
        stbi_write_png(filename.c_str(), width, height, nrComponents, data, 0);
        unsigned int textureID = sendTextureToGPU(data, width, height, nrComponents);
        return textureID;
    }
}

unsigned int Shared::generateMetallicTexture()
{
    unsigned char pixel[4] = {0, 0, 0, 0};
    return Shared::generateTexture(pixel);
}
unsigned int Shared::generateNonMetallicTexture()
{
    unsigned char pixel[4] = {255, 255, 255, 255};
    return Shared::generateTexture(pixel);
}
unsigned int Shared::generateWhiteAOTexture()
{
    unsigned char pixel[4] = {255, 255, 255, 255};
    return Shared::generateTexture(pixel);
}

namespace Shared {
    unsigned int generateDefaultRoughnessTexture()
    {
        // Default to fully rough.
        // Roughness convention used by the PBR shader:
        //   0.0 = smooth/mirror-like, 1.0 = fully rough (very blurred reflections).
        // Roughness is sampled from the red channel in the PBR shader.
        unsigned char pixel[4] = {255, 255, 255, 255};
        return Shared::generateTexture(pixel);
    }
}
unsigned int Shared::generateFlatNormalTexture()
{
    unsigned char pixel[4] = {128, 128, 255, 255};
    return Shared::generateTexture(pixel);
}

void Shared::WriteTextFile(const fs::path& p, const std::string& s)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    out.write(s.data(), static_cast<std::streamsize>(s.size()));
}

unsigned int Shared::generateTexture(unsigned char* pixel)
{
unsigned int emptyTexture;
glGenTextures(1, &emptyTexture);
glBindTexture(GL_TEXTURE_2D, emptyTexture);

// Define a single black pixel (RGBA)
// unsigned char blackPixel[4] = {255, 255, 255, 255}; // Black and fully transparent
// Alternatively: {255, 255, 255, 255} for white and opaque

// Allocate the texture with this single pixel
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

// Set texture parameters to avoid sampling artifacts
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

glBindTexture(GL_TEXTURE_2D, 0); // Unbind
return emptyTexture;
}

void Shared::readAnimation(std::string filename)
{
    auto animation = new Animation(filename);
    getUIState().animations.push_back(animation);
    getUIState().animationNames.push_back(animation->animationName);
}

bool Shared::endsWith(const std::string& value, const std::string& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void  Shared::initGpuLogger()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(Shared::glDebugOutput, nullptr);
}

GLFWwindow*  Shared::InitBackEndsWithWindow()
{
    auto window = Shared::initAWindow(EngineState::state->isVSyncOn);
    Shared::initImguiBackend(window);
    EngineState::state->engineRegistry->init();
    getPhysicsSystem().Init();

    TracyGpuContext;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // disable mouse pointer

    return window;
}

GLFWwindow* Shared::initAWindow(bool isVsyncOn)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    const int initialWidth = mode->width;
    const int initialHeight = mode->height;
    GLFWwindow* window = glfwCreateWindow(initialWidth, initialHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (window == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return nullptr;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(window);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));


    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSwapInterval(isVsyncOn); //V-sync off

    return window;
}

void Shared::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust viewport when the window is resized
    glViewport(0, 0, width, height);
}

void Shared::initImguiBackend(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // IMPORTANT: Do not let ImGui install GLFW callbacks.
    // The engine uses its own per-window input callbacks (InputHandler::mouse_button_callback etc.)
    // and routes events to ImGui explicitly.
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 130"); // Replace with your GLSL version

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();
}

ImGuiContext* Shared::createImguiContext()
{
    IMGUI_CHECKVERSION();
    return ImGui::CreateContext();
}

ImNodesContext* Shared::createImNodesContext()
{
    return ImNodes::CreateContext();
}

void Shared::initImguiBackendForWindow(GLFWwindow* window)
{
    // IMPORTANT:
    // - This initializes backends for the *current* ImGui context.
    // - Do not call this twice for the same (context, window) pair.
    // - Do not use this on the main Editor window if it was already initialized
    //   by Shared::InitBackEndsWithWindow()/Shared::initImguiBackend().
    if (!window)
        return;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // See initImguiBackend(): we keep callbacks under engine control.
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();
}

void Shared::shutdownImguiBackendForWindow()
{
    // Shutdown the backends for the current ImGui context.
    // Note: imgui_impl_glfw stores backend data via windows properties on Win32.
    // Shutting down the correct backend/context pair is required to avoid crashes.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void APIENTRY Shared::glDebugOutput(const GLenum source, const GLenum type, const GLuint id, const GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    // Only keep HIGH severity
    if (severity != GL_DEBUG_SEVERITY_HIGH) return;

    static std::unordered_set<GLuint> g_seenIds;

    // Optional: ignore known noisy IDs (examples vary by driver)
    // if (id == 131185 || id == 131204) return;

    auto [_, inserted] = g_seenIds.insert(id);
    if (!inserted) return; // already logged this ID

    std::cerr << "\n=== OpenGL HIGH severity ===\n"
              << "ID: " << id << "\n"
              << "Source: " << source << "  Type: " << type << "\n"
              << "Msg: " << message << "\n"
              << "============================\n";

    assert(false && "OPEN ERRORS");
}

fs::path Shared::metaFileToActualPath(const fs::path& path)
{
    auto justMetaGuid = path.stem().stem().string();
    return path.parent_path() / fs::path(getEngineRegistryFilesMap()[justMetaGuid]).filename();
}

void Shared::CopyFileToProjectDirectory(std::string& filelocation)
{
    if (filelocation.empty())
        return;

    auto projectDirectory = fs::path(EngineState::state->currentActiveProjectDirectory);
    auto filePath = fs::path(filelocation);

    // 1. Define your target destination: /projectDirectory/Assets/
    fs::path assetsDir = projectDirectory / "Assets";

    // 2. Ensure the Assets directory actually exists
    if (!fs::exists(assetsDir)) {
        fs::create_directories(assetsDir);
    }

    // 3. Check if the file is already inside the project directory
    // Use absolute paths to avoid confusion with relative pathing
    auto absoluteProject = fs::absolute(projectDirectory);
    auto absoluteFile = fs::absolute(filePath);

    // Search for the project path within the file path
    auto rel = std::search(absoluteFile.begin(), absoluteFile.end(),
                           absoluteProject.begin(), absoluteProject.end());

    bool isInsideProject = (rel != absoluteFile.end());

    if (!isInsideProject) {
        try {
            fs::path destination = assetsDir / filePath.filename();
            fs::copy_file(filePath, destination, fs::copy_options::overwrite_existing);
            filelocation = destination.string();

            std::cout << "File copied to: " << destination << std::endl;
        } catch (fs::filesystem_error& e) {
            auto temp = !filePath.empty() ? filePath: "FILE_PATH_EMPTY. ";
            std::cerr << "Could not copy file: " << temp << "Error: "  << e.what() << std::endl;
        }
    } else {
        std::cout << "File is already within the project directory." << std::endl;
    }
}


std::string Shared::uuid()
{
    return boost::uuids::to_string(boost::uuids::random_generator()());
}