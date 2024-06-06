// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <windows.h>

#include <shader.hpp>
#include <Input.hpp>
#include <Camera.hpp>
#include <light.hpp>

#include "model.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>

#include "outliner.hpp"

#include "raypicking.hpp"

#include <state.hpp>
#include "cubemap.hpp"

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

struct ClientHandler {
    InputHandler* inputHandler;
    Camera* camera;

} clientHandler;

State* State::state = new State();

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    std::cerr << "GL Callback: " << message << std::endl;
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

    unsigned int mouseState = GLFW_CURSOR_DISABLED;
    glfwSetInputMode(mWindow, GLFW_CURSOR, mouseState); // disable mouse pointer
    // stbi_set_flip_vertically_on_load(true);

    //Init clienthandler
    clientHandler.camera = new Camera();
    clientHandler.inputHandler = new InputHandler(clientHandler.camera, mWindow, 800, 600);
    InputHandler::currentInputHandler = clientHandler.inputHandler;

    //CubeMap -- Blocking 0th textureId for environment map. Models will start using from 1+ index.
    auto cubeMap = new CubeMap("E:/OpenGL/Models/quarry_cloudy_4k.hdr");
    auto equirectangularToCubemapShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/cubemap.vert","E:/OpenGL/Glitter/Glitter/Shaders/equirectanglular_to_cubemap.frag");
    auto irradianceShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/cubemap.vert","E:/OpenGL/Glitter/Glitter/Shaders/irradiance_convolution.frag");
    auto backgroundShader = new Shader("E:/OpenGL/Glitter/Glitter/Shaders/background.vert","E:/OpenGL/Glitter/Glitter/Shaders/background.frag");
    cubeMap->setup(mWindow,
    *equirectangularToCubemapShader, *irradianceShader);

    //Load modal
    auto shader1 =  new Shader("E:/OpenGL/Glitter/Glitter/Shaders/basic.vert","E:/OpenGL/Glitter/Glitter/Shaders/pbr.frag");
    shader1->use();
    shader1->setInt("irradianceMap", 0);
    // auto shader2 =  new Shader("E:/OpenGL/Glitter/Glitter/Shaders/basic.vert","E:/OpenGL/Glitter/Glitter/Shaders/basic.frag");
    std::vector<Shader*> shaders = {shader1};
    auto rayCastshader =  new Shader(
        "E:/OpenGL/Glitter/Glitter/Shaders/rayCast.vert",
        "E:/OpenGL/Glitter/Glitter/Shaders/rayCast.frag");
    // auto model3d = new Model("E:/OpenGL/backpack/backpack.obj");
    
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //Start loading a 3D model here ?
    auto models = new std::vector<Model*>();

    // auto model3d = new Model("E:/OpenGL/Models/Paladin J Nordstrom.fbx");
    // model3d->model = glm::scale(model3d->model, glm::vec3(.03,.03,.03));
    // models->push_back(model3d);

    auto model3d2 = new Model("E:/OpenGL/Models/sphere.fbx");
    // E:\OpenGL\Models\rustediron1-alt2-Unreal-Engine\rustediron1-alt2-Unreal-Engine
    model3d2->LoadTexture("E:/OpenGL/Models/rustediron1-alt2-Unreal-Engine/rustediron1-alt2-Unreal-Engine/rustediron2_basecolor.png", aiTextureType_DIFFUSE);
    model3d2->LoadTexture("E:/OpenGL/Models/rustediron1-alt2-Unreal-Engine/rustediron1-alt2-Unreal-Engine/rustediron2_normal.png", aiTextureType_NORMALS);
    model3d2->LoadTexture("E:/OpenGL/Models/rustediron1-alt2-Unreal-Engine/rustediron1-alt2-Unreal-Engine/rustediron2_metallic.png", aiTextureType_METALNESS);
    model3d2->LoadTexture("E:/OpenGL/Models/rustediron1-alt2-Unreal-Engine/rustediron1-alt2-Unreal-Engine/rustediron2_roughness.png", aiTextureType_DIFFUSE_ROUGHNESS);
    model3d2->LoadTexture("E:/OpenGL/Models/rustediron1-alt2-Unreal-Engine/rustediron1-alt2-Unreal-Engine/whiteAO.png", aiTextureType_AMBIENT_OCCLUSION);
    models->push_back(model3d2);

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

    auto outliner = new Outliner(models);

    glm::vec3 rayOrigin, rayDir;

    //GPULogger
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glDebugOutput, nullptr);

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

        // render the model
        for(int i=0;i<models->size();i++)
        {
            glm::vec3 position((*models)[i]->model[3][0], (*models)[i]->model[3][1], (*models)[i]->model[3][2]);
            shaders.at(i)->use();
            clientHandler.camera->updateMVP(shaders.at(i)->ID);
            glUniformMatrix4fv(glGetUniformLocation(shaders.at(i)->ID, "model"), 1, GL_FALSE, glm::value_ptr((*models)[i]->model));
            glUniform3f(glGetUniformLocation(shaders.at(i)->ID, "viewPos"), clientHandler.camera->getPosition().r, clientHandler.camera->getPosition().g, clientHandler.camera->getPosition().b);

            // This is a spotlight attached to the client's camera
            // lights->spotLights[0].position = clientHandler.camera->getPosition();
            // lights->spotLights[0].direction = clientHandler.camera->getFront();
            
            //Passing values required by the shader for the lights present in the scene
            lights->Render(shaders.at(i)->ID);
            (*models)[i]->Draw(shaders.at(i), mWindow);
        }

        // equirectangularToCubemapShader->use();
        // equirectangularToCubemapShader->setMat4("view", clientHandler.camera->viewMatrix());
        // equirectangularToCubemapShader->setMat4("projection", clientHandler.camera->projectionMatrix());
        cubeMap->Draw(clientHandler.camera->viewMatrix(), clientHandler.camera->projectionMatrix(), *backgroundShader);

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
            *models,
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
        (*models)[getSelectedIndex]->imguizmoManipulate(clientHandler.camera->viewMatrix(), (clientHandler.camera->projectionMatrix()));

        //Render the outliner
        outliner->Render();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }   glfwTerminate();
    return EXIT_SUCCESS;
}
