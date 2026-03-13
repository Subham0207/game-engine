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
#include "../Headers/UI/AssetBrowser/AssetBrowser.hpp"

#include "Helpers/raypicking.hpp"

#include <EngineState.hpp>
#include "Lights/cubemap.hpp"
#include "Level/Level.hpp"
#include <Helpers/Shared.hpp>
#include <Sprites/text.hpp>

#include <PhysicsSystem.hpp>
#include <UI/PropertiesPanel.hpp>
#include <Controls/PlayerController.hpp>

#include "Camera/FlyCam.hpp"
#include "Controls/ClientHandler.hpp"
#include "Event/EventBus.hpp"
#include "Event/EventQueue.hpp"
#include "Event/InputContext.hpp"
#include "RenderPipeline/LightingPass.hpp"
#include "RenderPipeline/PostProcess.hpp"
#include "RenderPipeline/ShadowPass.hpp"

#include <Profiler.hpp>

#include "ProjectManifest.hpp"
#include "Debug/Raycast.hpp"
#include "Helpers/GetExecutablePath.hpp"
#include "Lights/Skybox.hpp"


int Editor::openEditor() {

    EventQueue queue;
    InputContext inputCtx;
    inputCtx.queue = &queue;

    auto state = new EngineState();
    state->init();
    EngineState::state = state;

    ClientHandler::clientHandler = new ClientHandler();

    LuaRegistry::SetupLua(EngineState::state->luaEngine->state(), EngineState::state->currentActiveProjectDirectory);

    // Load GLFW and Create a Window
    EngineState::state->mWindow = Shared::InitBackEndsWithWindow();
    auto& mWindow = EngineState::state->mWindow;

    //Loading Level -- making .lvl as the extention of my levelfile
    auto level = new Level();
    EngineState::state->activeLevel = level; //Correct active level before loading a save level is important for rendererable to get to correct array.
    auto lvl = EngineState::state->activeLevel;
    lvl->cameras.push_back(EngineState::state->editorCamera);

    //Init clienthandler
    auto camera = lvl->cameras[EngineState::state->activeCameraIndex];
    ClientHandler::clientHandler->inputHandler = new InputHandler(camera, mWindow, 800, 600);
    InputHandler::currentInputHandler = ClientHandler::clientHandler->inputHandler;

    level->loadMainLevelOfCurrentProject();

    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);

    auto skyBox = new Lighting::Skybox(engineFSPath, mWindow);

    //Generate textureIds for Some Default texture
    EngineState::state->GenerateDefaultMaterials();

    auto rayCastObjectSelector = new Debug::Raycast(engineFSPath);

    //Lights setup
    auto lights = new Lights();
    lights->initDefaultLights();

    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    auto outliner = new Outliner();
    auto assetBrowser = new ProjectAsset::AssetBrowser();

    Controls::PlayerController::register_bindings(getLuaEngine());

    Shared::initGpuLogger();

    ShadowPass shadowPass(mWindow, lights);
    LightingPass lightingPass{};
    PostProcess postProcess{};

    EngineState::state->postProcess = &postProcess;

    bool firstFrame = false;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {

        FrameMark;
        ZoneScopedN("Engine Frame");

        queue.drain([&](const Event& e)
        {
            EngineState::state->bus.dispatch(e);
        });

        //delta time -- making things time dependent
        auto activeCamera = InputHandler::currentInputHandler->m_Camera;
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - EngineState::state->lastFrame;
        EngineState::state->deltaTime = deltaTime;
        EngineState::state->lastFrame = currentFrame;

        auto& activeLevel = getActiveLevel();
        auto& lvlrenderables = activeLevel.renderables;

        ClientHandler::clientHandler->inputHandler->handleInput(EngineState::state->deltaTime, inputCtx);
        if(EngineState::state->isPlay)
        {
            ZoneScopedN("EditorIsPlay");
            // *activeCamera = lvl->cameras[EngineState::state->activePlayerControllerId + 1];
            if (!EngineState::state->playerControllers.empty())
            if (auto character = EngineState::state->playerControllers[EngineState::state->activePlayerControllerId]->getCharacter())
            {
                activeCamera = character->camera;
                ClientHandler::clientHandler->inputHandler->m_Camera = activeCamera;
            }

            //TODO: Remove below code after removing syncTransformationToPhysicsEntity() from Model class. And handle this functionality in same way as Character class does.
            //Update transform of physics enabled renderables
            //How do we get the transforms for a objects from the physics engine --- by its id i would guess
            if(getPhysicsSystem().isFirstPhysicsEnabledFrame == true)
            {
                //make the physics objects transformations ( including scale ) same as thier model
                getPhysicsSystem().isFirstPhysicsEnabledFrame = false;

                for(int i=0;i<lvlrenderables.size();i++)
                {
                    lvlrenderables.at(i)->syncTransformationToPhysicsEntity();
                }
            }
            else
            {
                getPhysicsSystem().Update(EngineState::state->deltaTime);

                for(int i=0;i<lvlrenderables.size();i++)
                {
                    lvlrenderables.at(i)->physicsUpdate();
                }
            }
        }
        else
        {
            InputHandler::currentInputHandler->m_Camera = lvl->cameras[0];

            getPhysicsSystem().isFirstPhysicsEnabledFrame = true;
        }

        postProcess.attachFBO();

        activeCamera->tick();

        skyBox->Draw(activeCamera->viewMatrix(), activeCamera->projectionMatrix());

        //TODO: move the below light selection in Lights class.
        for(auto &i: lights->pointLights)
        {
            ZoneScopedN("EditorPointLightSelection");
            i.position = i.lightModel->GetPosition();
            if(i.lightModel->getIsSelected())
            {
                //pass properties to properties pannel.
                getUIState().propretiesPanel->pointLight = &i;
            }
        }

        for(auto &i: lights->directionalLights)
        {
            ZoneScopedN("EditorPointDirectionalLightSelection");
            // GET direction vector from light model rotation.
            if(i.lightModel->getIsSelected())
            {
                //pass properties to properties pannel.
                getUIState().propretiesPanel->directionalLight = &i;
            }
        }

        for(auto &i: lights->spotLights)
        {
            ZoneScopedN("EditorPointSpotLightSelection");
            i.position = i.lightModel->GetPosition();
            if(i.lightModel->getIsSelected())
            {
                //pass properties to properties pannel.
                getUIState().propretiesPanel->spotlight = &i;
            }
        }

        //TODO: move below logic in level tick.
        if(!getActiveLevel().isNavMeshSetup)
        {
            ZoneScopedN("EditorOneTimeLevelNavMeshSetup");
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
            getActiveLevel().renderDebugNavMesh(activeCamera);
        }

        // getActiveLevel().renderLevelvertices(activeCamera);

        getActiveLevel().tickAIs(EngineState::state->deltaTime);

        //Update animation before Shadow pass and Lighting pass to get correct shadows and lighting.
        for(int i=0;i<lvlrenderables.size();i++)
        {
            ZoneScopedN("EditorUpdateBoneMatrixOnCPU");
            if(lvlrenderables.at(i)->ShouldRender())
            {
                if (auto character = std::dynamic_pointer_cast<Character>(lvlrenderables.at(i)))
                {
                    if(character->animator)
                    {
                        character->updateFinalBoneMatrix(deltaTime);
                    }
                }
            }
        }

        postProcess.draw(
            shadowPass,
            lightingPass,
            lvlrenderables,
            activeCamera,
            lights,
            skyBox->getCubeMap(),
            deltaTime
            );

        for(int i=0;i<getActiveLevel().textSprites.size();i++)
        {
            getActiveLevel().textSprites.at(i)->RenderText3D(activeCamera->viewMatrix(), activeCamera->projectionMatrix());
        }


        //Thinking imgui should be last in call chain to show up last on screen ??
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (EngineState::state->isDevMode)
        {
            rayCastObjectSelector->HandleSelection(
                outliner,
                activeCamera,
                &activeLevel);

            //Render the outliner
            outliner->Render(*lvl);
            assetBrowser->RenderAssetBrowser();
        }

        {
            ZoneScopedN("ImGuiRender");
            ImGui::Render();
        }
        {
            ZoneScopedN("ImGuiRenderDrawData");
            {
                TracyGpuZone("ImGui Draw");
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
        }

        {
            ZoneScopedN("SwapBuffers");
            // Flip Buffers and Draw
            glfwSwapBuffers(mWindow);
        }

        TracyGpuCollect;

        {
            ZoneScopedN("GLFWPollEvents");
            glfwPollEvents();
        }

        if (!firstFrame)
        {
            // Cannot update the camera in middle of frame since ImGui still holds the NDC of the editorCamera. So waiting for first frame to end.
            if (!EngineState::state->isDevMode) EngineState::state->isPlay = true;
            firstFrame = true;
        }

    }   glfwTerminate();
    return EXIT_SUCCESS;
}
