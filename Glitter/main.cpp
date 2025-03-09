// Local Headers
#include "Helpers/glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <windows.h>

#include "Helpers/shader.hpp"
#include "Controls/Input.hpp"
#include "Camera/Camera.hpp"
#include "Lights/light.hpp"

#include "3DModel/model.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>

#include "UI/outliner.hpp"
#include "UI/AssetBrowser.hpp"

#include "Helpers/raypicking.hpp"
#include <ImGuizmo.h>

#include <EngineState.hpp>
#include "Lights/cubemap.hpp"
#include "Level/Level.hpp"
#include <Helpers/Shared.hpp>
#include <Sprites/text.hpp>

#include <Controls/statemachine.hpp>

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

//This is default stuff
struct ClientHandler {
    InputHandler* inputHandler;
    Camera* camera;

} clientHandler;

State* State::state = new State();

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    // std::cerr << "GL Callback: " << message << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust viewport when the window is resized
    mWidth = width;
    mHeight = height;
    glViewport(0, 0, width, height);
}

int main(int argc, char * argv[]) {

    char cwd[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, cwd)) {
        std::cout << "Current working dir: " << cwd << std::endl;
    } else {
        std::cerr << "Failed to get current working directory." << std::endl;
    }

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    mWidth = mode->width;
    mHeight = mode->height;
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
    

    glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);

    unsigned int mouseState = GLFW_CURSOR_DISABLED;
    glfwSetInputMode(mWindow, GLFW_CURSOR, mouseState); // disable mouse pointer
    // stbi_set_flip_vertically_on_load(true);

    //Init clienthandler
    clientHandler.camera = new Camera();
    clientHandler.inputHandler = new InputHandler(clientHandler.camera, mWindow, 800, 600);
    InputHandler::currentInputHandler = clientHandler.inputHandler;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //Loading Level -- making .lvl as the extention of my levelfile
    auto lvl = new Level();
    State::state->activeLevel = *lvl;
    lvl->camera = clientHandler.camera;
    lvl->cameraUp = &clientHandler.camera->cameraUp;
    lvl->cameraFront = &clientHandler.camera->cameraFront;
    lvl->cameraPos = &clientHandler.camera->cameraPos;
    // if(Level::checkIfLevelFileExists("Levels/Level1.lvl"))
    // {
    //     Level::loadFromFile("Levels/Level1.lvl", *lvl);
    // }
    
    //CubeMap -- Blocking 0th textureId for environment map. Models will start using from 1+ index.
    auto cubeMap = new CubeMap("E:/OpenGL/Models/rostock_laage_airport_8k.hdr");
    auto equirectangularToCubemapShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/cubemap.vert","E:/OpenGL/Glitter/Glitter/Shaders/equirectanglular_to_cubemap.frag");
    auto irradianceShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/cubemap.vert","E:/OpenGL/Glitter/Glitter/Shaders/irradiance_convolution.frag");
    auto prefilterShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/cubemap.vert","E:/OpenGL/Glitter/Glitter/Shaders/prefilter.frag");
    auto brdfShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/brdf.vert","E:/OpenGL/Glitter/Glitter/Shaders/brdf.frag");
    auto backgroundShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/background.vert","E:/OpenGL/Glitter/Glitter/Shaders/background.frag");
    cubeMap->setup(mWindow,
    *equirectangularToCubemapShader, *irradianceShader, *prefilterShader, *brdfShader);

    //For RGBA to work Enable Alpha channel and Blend; 
    //NOTE: Its very important to enable alpha and blend after cubemap generation else brdfLUT will come out black
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Generate textureIds for Some Default texture
    getUIState().metalicTextureID = Shared::generateMetallicTexture();
    getUIState().nonMetalicTextureID = Shared::generateMetallicTexture();
    getUIState().whiteAOTextureID = Shared::generateWhiteAOTexture();


    //Create different shaders for the each model
    
    auto rayCastshader =  new Shader(
        "E:/OpenGL/Glitter/Glitter/Shaders/rayCast.vert",
        "E:/OpenGL/Glitter/Glitter/Shaders/rayCast.frag");
    // auto model3d = new ModelType("E:/OpenGL/backpack/backpack.obj");
    
    //Lights setup
    auto lights = new Lights(); //for PBR removing directional lights and spotlight; Set them back up later.
    // lights->directionalLights.push_back(DirectionalLight(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)));

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };
    for (unsigned int i = 0; i < 4; i++)
    {
        lights->pointLights.push_back(PointLight(pointLightPositions[i], glm::vec3(1.0f,0.7f,0.7f)));
    }

    //Something wrong with spotlights only then; 
    // So right now I don't know how to pass dynamic array sizes to the shader. This caused an issue where if I statically
    // allocated memory for 4 spotlights and passed less  than 4 spotlights. The ones which is not passed makes the result 0
    // lights->spotLights = {
    //     SpotLight(glm::vec3(0.0f,0.0f,3.0f), glm::vec3(0.0f,0.4f,0.8f))
    // };

    //Start loading a 3D model here ?
    auto renderables = lvl->renderables;
    
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    //imgui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 

    ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
    ImGui_ImplOpenGL3_Init("#version 130"); // Replace with your GLSL version


    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends

    auto outliner = new Outliner(*renderables);
    auto assetBrowser = new ProjectAsset::AssetBrowser();

    glm::vec3 rayOrigin, rayDir;

    //GPULogger
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glDebugOutput, nullptr);

    //Init
    Shared::readAnimation("E:/OpenGL/Models/Idle.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Standard Walk.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Running.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Jumping.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Jog Strafe Left.fbx");
    Shared::readAnimation("E:/OpenGL/Models/Jog Strafe Right.fbx");

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        //delta time -- making things time dependent
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        clientHandler.inputHandler->handleInput(deltaTime);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        cubeMap->Draw(clientHandler.camera->viewMatrix(), clientHandler.camera->projectionMatrix(), *backgroundShader);

        // render the model
        for(int i=0;i<renderables->size();i++)
        {
            renderables->at(i)->useAttachedShader();
            (*renderables)[i]->draw(deltaTime, clientHandler.camera, lights, cubeMap);
        }

        for(int i=0;i<getActiveLevel().textSprites.size();i++)
        {
            getActiveLevel().textSprites.at(i)->RenderText3D(clientHandler.camera->viewMatrix(), clientHandler.camera->projectionMatrix());
        }
        

        //Thinking imgui should be last in call chain to show up last on screen ??
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuizmo::BeginFrame();
        // Set the window and matrix for ImGuizmo
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
        
        auto getSelectedIndex = outliner->GetSelectedIndex();
        rayCastshader->use();
        clientHandler.camera->updateMVP(rayCastshader->ID);
        auto getSelectedIndexFromMouseCurrentFrame = handlePicking(
            InputHandler::currentInputHandler->lastX,
            InputHandler::currentInputHandler->lastY,
            *renderables,
            InputHandler::currentInputHandler->m_Camera->viewMatrix(),
            InputHandler::currentInputHandler->m_Camera->projectionMatrix(),
            rayCastshader->ID,
            rayOrigin,
            rayDir,
            InputHandler::currentInputHandler->m_Camera->getCameraLookAtDirectionVector()
        );
        if(getSelectedIndexFromMouseCurrentFrame > -2)
        outliner->setSelectedIndex(getSelectedIndexFromMouseCurrentFrame);


        if(getSelectedIndex > -1)
        (*renderables)[getSelectedIndex]->imguizmoManipulate(clientHandler.camera->viewMatrix(), (clientHandler.camera->projectionMatrix()));

        //Render the outliner
        outliner->Render(*lvl);
        assetBrowser->RenderAssetBrowser();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}
