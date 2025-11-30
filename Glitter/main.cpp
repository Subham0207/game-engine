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

#include <PhysicsSystem.hpp>
#include <Physics/Box.hpp>
#include <Physics/capsule.hpp>
#include <UI/ProjectManager.hpp>
#include <UI/PropertiesPanel.hpp>
#include <AI/AI.hpp>

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
GLFWwindow *mWindow;

//This is default stuff
struct ClientHandler {
    InputHandler* inputHandler;
} clientHandler;

EngineState* EngineState::state = new EngineState();

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust viewport when the window is resized
    mWidth = width;
    mHeight = height;
    glViewport(0, 0, width, height);
}

int initAWindow();
void imguiBackend()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 

    ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
    ImGui_ImplOpenGL3_Init("#version 130"); // Replace with your GLSL version

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
}
int openEditor();
int main(int argc, char * argv[])
{
    if (fs::exists(fs::path(EngineState::state->engineInstalledDirctory) / "user_prefs.json")) {
        std::ifstream infile(fs::path(EngineState::state->engineInstalledDirctory) / "user_prefs.json");
        std::string line;
        while (std::getline(infile, line)) {
            // Check if the line is not empty before adding.
            if (!line.empty()) {
                getUIState().recent_projects.push_back(fs::path(line));
            }
        }
        infile.close();
    }

    initAWindow();

    imguiBackend();

    while (glfwWindowShouldClose(mWindow) == false)
    {

        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //////Code changes go here//////
        UI::projectManager();
        ////////////////////////////////

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(mWindow);
        glfwPollEvents();

         // Now it's safe to leave the loop
        if(EngineState::state->currentActiveProjectDirectory != "")
        break;

    }   
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();


    if(EngineState::state->currentActiveProjectDirectory != "")
    {
        deltaTime = 0.0f;
        lastFrame = 0.0f;
        return openEditor();
    }
    
    return EXIT_SUCCESS;
}

int initAWindow()
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

    mWidth = mode->width;
    mHeight = mode->height;
    mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

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
}

int openEditor() {

    char cwd[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, cwd)) {
        std::cout << "Current working dir: " << cwd << std::endl;
    } else {
        std::cerr << "Failed to get current working directory." << std::endl;
    }

    // Load GLFW and Create a Window
    initAWindow();
    imguiBackend();
    EngineState::state->engineRegistry->init();
    getPhysicsSystem().Init();

    unsigned int mouseState = GLFW_CURSOR_DISABLED;
    glfwSetInputMode(mWindow, GLFW_CURSOR, mouseState); // disable mouse pointer
    // stbi_set_flip_vertically_on_load(true);

    //Loading Level -- making .lvl as the extention of my levelfile
    auto level = new Level();
    EngineState::state->activeLevel = level; //Correct active level before loading a save level is important for rendererable to get to correct array.
    auto defaultCamera = new Camera("defaultcamera");
    auto lvl = EngineState::state->activeLevel;
    // State::state->activeLevel = new level(); state already has a new level initialized
    lvl->cameras.push_back(defaultCamera);
    
    //Init clienthandler
    auto camera = lvl->cameras[EngineState::state->activeCameraIndex];
    clientHandler.inputHandler = new InputHandler(camera, mWindow, 800, 600);
    InputHandler::currentInputHandler = clientHandler.inputHandler;
    
    level->loadMainLevelOfCurrentProject();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // if(Level::checkIfLevelFileExists("Levels/Level1.lvl"))
    // {
    //     Level::loadFromFile("Levels/Level1.lvl", *lvl);
    // }
    
    //CubeMap -- Blocking 0th textureId for environment map. Models will start using from 1+ index.
    auto cubeMap = new CubeMap("./EngineAssets/rostock_laage_airport_8k.hdr");
    auto equirectangularToCubemapShader = new Shader("./Shaders/cubemap.vert","./Shaders/equirectanglular_to_cubemap.frag");
    auto irradianceShader = new Shader("./Shaders/cubemap.vert","./Shaders/irradiance_convolution.frag");
    auto prefilterShader = new Shader("./Shaders/cubemap.vert","./Shaders/prefilter.frag");
    auto brdfShader = new Shader("./Shaders/brdf.vert","./Shaders/brdf.frag");
    auto backgroundShader = new Shader("./Shaders/background.vert","./Shaders/background.frag");
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
    getUIState().flatNormalTextureID= Shared::generateFlatNormalTexture();


    //Create different shaders for the each model
    
    auto rayCastshader =  new Shader(
        "./Shaders/rayCast.vert",
        "./Shaders/rayCast.frag");
    
    //Lights setup
    auto lights = new Lights(); //for PBR removing directional lights and spotlight; Set them back up later.
    // lights->directionalLights.push_back(DirectionalLight(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)));

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  2.0f,  2.0f),
        glm::vec3(2.3f, 2.0f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  2.0f, -3.0f)
    };

    glm::vec3 directionLightPositions[] = {
        glm::vec3(0.7f,  2.0f,  3.0f),
    };

    glm::vec3 spotLightPositions[] = {
        glm::vec3(5.0f,  6.0f,  -1.0f),
    };
    for (unsigned int i = 0; i < 4; i++)
    {
        lights->pointLights.push_back(PointLight(pointLightPositions[i], glm::vec3(0.0f,1.0f,0.0f)));
    }

    for (unsigned int i = 0; i < 1; i++)
    {
        lights->directionalLights.push_back(DirectionalLight(directionLightPositions[i], glm::vec3(0.0f,-1.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f)));
    }

    for (unsigned int i = 0; i < 1; i++)
    {
        lights->spotLights.push_back(
            SpotLight(
                spotLightPositions[i],
                glm::vec3(1.0f,0.0f,0.0f),
                glm::vec3(0.0f, -1.0f, 0.0f),
                40.0f,
                70.0f
            )
        );
    }


    auto renderables = lvl->renderables;
    
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    auto outliner = new Outliner(*renderables);
    auto assetBrowser = new ProjectAsset::AssetBrowser();

    auto aiCharacter = new Character("./EngineAssets/Aj.fbx");
    aiCharacter->model->setTransform(glm::vec3(0.0f,3.0f,1.0f),glm::quat(), glm::vec3(0.03f,0.03,0.03));
    aiCharacter->capsuleColliderPosRelative = glm::vec3(0.0f,-2.5f,0.0f);
    auto playerController = aiCharacter->playerController;
    getActiveLevel().addRenderable(aiCharacter);
    auto ai = new AI::AI(playerController);

    glm::vec3 rayOrigin, rayDir;

    //GPULogger
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glDebugOutput, nullptr);

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
                
        //delta time -- making things time dependent
        auto activeCamera = &InputHandler::currentInputHandler->m_Camera;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        auto lvlrenderables = getActiveLevel().renderables;

        clientHandler.inputHandler->handleInput(deltaTime);
        if(EngineState::state->isPlay)
        {
            *activeCamera = lvl->cameras[EngineState::state->activePlayerControllerId + 1];

            //Update transform of physics enabled renderables
            //How do we get the transforms for a objects from the physics engine --- by its id i would guess
            if(getPhysicsSystem().isFirstPhysicsEnabledFrame == true)
            {
                //make the physics objects transformations ( including scale ) same as thier model
                getPhysicsSystem().isFirstPhysicsEnabledFrame = false;
                
                for(int i=0;i<lvlrenderables->size();i++)
                {
                    lvlrenderables->at(i)->syncTransformationToPhysicsEntity();
                }
            }
            else
            {
                getPhysicsSystem().Update(deltaTime);

                for(int i=0;i<lvlrenderables->size();i++)
                {
                    lvlrenderables->at(i)->physicsUpdate();
                }
            }
        }
        else
        {
            InputHandler::currentInputHandler->m_Camera = lvl->cameras[0];
            
            getPhysicsSystem().isFirstPhysicsEnabledFrame = true;
        }

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        cubeMap->Draw((*activeCamera)->viewMatrix(), (*activeCamera)->projectionMatrix(), *backgroundShader);

        for(auto &i: lights->pointLights)
        {
            i.position = i.lightModel->GetPosition();
            if(i.lightModel->getIsSelected())
            {
                //pass properties to properties pannel.
                getUIState().propretiesPanel->pointLight = &i;
            }
        }

        for(auto &i: lights->directionalLights)
        {
            // GET direction vector from light model rotation.
            if(i.lightModel->getIsSelected())
            {
                //pass properties to properties pannel.
                getUIState().propretiesPanel->directionalLight = &i;
            }
        }

        for(auto &i: lights->spotLights)
        {
            i.position = i.lightModel->GetPosition();
            if(i.lightModel->getIsSelected())
            {
                //pass properties to properties pannel.
                getUIState().propretiesPanel->spotlight = &i;
            }
        }

        if(!getActiveLevel().isNavMeshSetup)
        {
            getActiveLevel().BuildLevelNavMesh();
            auto startingPosition = aiCharacter->GetPosition();
            std::vector<float> outPath;

            // float start[3] = {0.0f, 0.0f, 0.0f};
            // getActiveLevel().SampleRandomPoint(start);


            // getActiveLevel().FindPath(start, end, outPath);

            getActiveLevel().isNavMeshSetup = true;
        }

        // getActiveLevel().renderLevelvertices(*activeCamera);

        ai->Tick(deltaTime);

        lights->directionalLights[0].evaluateShadowMap(mWindow, deltaTime, *activeCamera, lights, cubeMap);
        lights->spotLights[0].evaluateShadowMap(mWindow);

        for(auto& light : lights->pointLights)
            light.evaluateShadowMap(mWindow);

        // render the model
        for(int i=0;i<lvlrenderables->size();i++)
        {
            if(lvlrenderables->at(i)->ShouldRender())
            {
                lvlrenderables->at(i)->useAttachedShader();
                glActiveTexture(GL_TEXTURE0 + 9);
                glBindTexture(GL_TEXTURE_2D, lights->directionalLights[0].shadowMap);
                (*lvlrenderables)[i]->draw(deltaTime, *activeCamera, lights, cubeMap);
            }
        }

        for(int i=0;i<getActiveLevel().textSprites.size();i++)
        {
            getActiveLevel().textSprites.at(i)->RenderText3D((*activeCamera)->viewMatrix(), (*activeCamera)->projectionMatrix());
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
        (*activeCamera)->updateMVP(rayCastshader->ID);
        auto getSelectedIndexFromMouseCurrentFrame = handlePicking(
            InputHandler::currentInputHandler->lastX,
            InputHandler::currentInputHandler->lastY,
            *lvlrenderables,
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
        (*lvlrenderables)[getSelectedIndex]->imguizmoManipulate((*activeCamera)->viewMatrix(), ((*activeCamera)->projectionMatrix()));

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
