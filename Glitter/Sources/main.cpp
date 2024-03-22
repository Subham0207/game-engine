// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>

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

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

struct ClientHandler {
    InputHandler* inputHandler;
    Camera* camera;

} clientHandler;

void SelectionBuffer(std::vector<Model *> *models, unsigned int FBO, Shader *shader, Outliner *outliner);

int main(int argc, char * argv[]) {

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


    //Load modal
    auto shader =  new Shader("E:/OpenGL/Glitter/Glitter/Shaders/basic.vert","E:/OpenGL/Glitter/Glitter/Shaders/basic.frag");
    //Picking shader
    auto pickingShader =  new Shader(
        "E:/OpenGL/Glitter/Glitter/Shaders/pickingShader/pickingShader.vert",
        "E:/OpenGL/Glitter/Glitter/Shaders/pickingShader/pickingShader.frag");
    // auto model3d = new Model("E:/OpenGL/backpack/backpack.obj");

    //Lights setup
    auto lights = new Lights();
    lights->directionalLights.push_back(DirectionalLight(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f)));

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };
    for (unsigned int i = 0; i < 4; i++)
    {
        lights->pointLights.push_back(PointLight(pointLightPositions[i], glm::vec3(1.0f,0.7f,0.0f)));
    }

    //Something wrong with spotlights only then; 
    // So right now I don't know how to pass dynamic array sizes to the shader. This caused an issue where if I statically
    // allocated memory for 4 spotlights and passed less  than 4 spotlights. The ones which is not passed makes the result 0
    lights->spotLights = {
        SpotLight(glm::vec3(0.0f,0.0f,3.0f), glm::vec3(0.0f,0.4f,0.8f))
    };

    glEnable(GL_DEPTH_TEST);

    //Start loading a 3D model here ?
    auto models = new std::vector<Model*>();

    auto model3d = new Model("E:/OpenGL/Models/Cottage/cottage_fbx.fbx");
    model3d->LoadTexture("E:/OpenGL/Models/Cottage/cottage_textures/cottage_diffuse.png", "texture_diffuse");
    model3d->pickColor[0] = 0;
    model3d->pickColor[1] = 1;
    model3d->pickColor[2] = 0;
    model3d->pickColor[3] = 0;
    models->push_back(model3d);

    auto model3d2 = new Model("E:/OpenGL/Models/Cottage/cottage_fbx.fbx");
    model3d2->pickColor[0] = 0;
    model3d2->pickColor[1] = 0;
    model3d2->pickColor[2] = 1;
    model3d2->pickColor[3] = 0;
    model3d2->LoadTexture("E:/OpenGL/Models/Cottage/cottage_textures/cottage_diffuse.png", "texture_diffuse");
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

    //FrameBuffer for selection
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);

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
            shader->use();
            clientHandler.camera->updateMVP(shader->ID);
            glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr((*models)[i]->model));
            glUniform3f(glGetUniformLocation(shader->ID, "viewPos"), clientHandler.camera->getPosition().r, clientHandler.camera->getPosition().g, clientHandler.camera->getPosition().b);
            glUniform1i(glGetUniformLocation(shader->ID, "material.diffuse"), 0);
            glUniform1i(glGetUniformLocation(shader->ID, "material.specular"), 1);
            glUniform1f(glGetUniformLocation(shader->ID, "material.shininess"), 32.0f);
            lights->spotLights[0].position = clientHandler.camera->getPosition();
            lights->spotLights[0].direction = clientHandler.camera->getFront();
            lights->Render(shader->ID);
            (*models)[i]->Draw(shader, mWindow);
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
        if(getSelectedIndex > -1)
        (*models)[getSelectedIndex]->imguizmoManipulate(clientHandler.camera->viewMatrix(), (clientHandler.camera->projectionMatrix()));

        SelectionBuffer(models, FBO, pickingShader, outliner);

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



void SelectionBuffer(std::vector<Model *> *models, unsigned int FBO, Shader *shader, Outliner *outliner)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set the texture's data store (adjust width, height, and format as necessary)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // Texture parameters - these can be adjusted as needed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

    // Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint depthRenderbuffer;
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mWidth, mHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

    // Check for completeness
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glViewport(0, 0, mWidth, mHeight);

    unsigned char pixel[4] = {0};

    // Clear the framebuffer
    glClearColor(0.0, 0.0, 0.0, 0.0); // Assuming black (or another color) means "no pick"
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i=0;i<models->size();i++)
    {
        glm::vec3 position((*models)[i]->model[3][0], (*models)[i]->model[3][1], (*models)[i]->model[3][2]);
        shader->use();
        clientHandler.camera->updateMVP(shader->ID);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr((*models)[i]->model));
        unsigned char* color = (*models)[i]->pickColor;
        GLfloat tempColor[4] = {
            color[0] / 255.0f, // R
            color[1] / 255.0f, // G
            color[2] / 255.0f, // B
            color[3] / 255.0f  // A
        };
        glUniform4fv(glGetUniformLocation(shader->ID, "pickColor"), 1, tempColor);
        (*models)[i]->Draw(shader);
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;

    glReadPixels(mousePos.x, mousePos.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);

    // Interpret the pixel as an object ID

    for(int i=0;i<models->size();i++)
    {
        // std::cout << "pickColor: " << (*models)[i]->pickColor << " and pixel color: "<< pixel << std::endl;
        if((*models)[i]->pickColor == pixel)
        {
            outliner->setSelectedIndex(i);
            std::cout << "Selected index: " << i << std::endl;
        }
    }

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}