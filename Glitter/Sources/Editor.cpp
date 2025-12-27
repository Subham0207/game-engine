//
// Created by subha on 20-12-2025.
//

#include "Editor.hpp"
#include "Helpers/glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
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

#include <utility>
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

#include <PhysicsSystem.hpp>
#include <UI/PropertiesPanel.hpp>
#include <Controls/PlayerController.hpp>

#include "Controls/ClientHandler.hpp"


int Editor::openEditor(std::string enginePath) {

    EngineState::state = new EngineState();
    EngineState::state->setCurrentDirAsProjectDirectory();
    EngineState::state->setEngineDirectory(std::move(enginePath));
    ClientHandler::clientHandler = new ClientHandler();

    char cwd[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, cwd)) {
        std::cout << "Current working dir: " << cwd << std::endl;
    } else {
        std::cerr << "Failed to get current working directory." << std::endl;
    }

    LuaRegistry::SetupLua(EngineState::state->luaEngine->state(), EngineState::state->currentActiveProjectDirectory);

    // Load GLFW and Create a Window
    auto mWindow = EngineState::state->mWindow;
    mWindow = Shared::initAWindow();
    Shared::initImguiBackend(mWindow);
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
    ClientHandler::clientHandler->inputHandler = new InputHandler(camera, mWindow, 800, 600);
    InputHandler::currentInputHandler = ClientHandler::clientHandler->inputHandler;

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

    glm::vec3 rayOrigin, rayDir;

    Controls::PlayerController::register_bindings(getLuaEngine());

    //GPULogger
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(Shared::glDebugOutput, nullptr);

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {

        //delta time -- making things time dependent
        auto activeCamera = &InputHandler::currentInputHandler->m_Camera;
        float currentFrame = glfwGetTime();
        EngineState::state->deltaTime = currentFrame - EngineState::state->lastFrame;
        EngineState::state->lastFrame = currentFrame;

        auto lvlrenderables = getActiveLevel().renderables;

        ClientHandler::clientHandler->inputHandler->handleInput(EngineState::state->deltaTime);
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
                getPhysicsSystem().Update(EngineState::state->deltaTime);

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
            std::vector<float> outPath;

            // float start[3] = {0.0f, 0.0f, 0.0f};
            // float end[3] = {0.0f, 0.0f, 0.0f};
            // getActiveLevel().SampleRandomPoint(start);
            // getActiveLevel().SampleRandomPoint(end);


            // getActiveLevel().FindPath(start, end, outPath);

            getActiveLevel().isNavMeshSetup = true;
        }
        else
        {
            if(getUIState().renderNavMesh)
            getActiveLevel().renderDebugNavMesh(*activeCamera);
        }

        // getActiveLevel().renderLevelvertices(*activeCamera);

        getActiveLevel().tickAIs(EngineState::state->deltaTime);

        lights->directionalLights[0].evaluateShadowMap(mWindow, EngineState::state->deltaTime, *activeCamera, lights, cubeMap);
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
                (*lvlrenderables)[i]->draw(EngineState::state->deltaTime, *activeCamera, lights, cubeMap);
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
        auto view = InputHandler::currentInputHandler->m_Camera->viewMatrix();
        auto proj = InputHandler::currentInputHandler->m_Camera->projectionMatrix();
        auto getSelectedIndexFromMouseCurrentFrame = handlePicking(
            InputHandler::currentInputHandler->lastX,
            InputHandler::currentInputHandler->lastY,
            *lvlrenderables,
            view,
            proj,
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
