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
#include <imnodes.h>

#include <utility>
#include <vector>

#include <algorithm>

#include <algorithm>

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

#include "Debug/Raycast.hpp"
#include "Lights/Skybox.hpp"
#include "NodeGraph/NodeGraph.hpp"

#include "Windowing/EditorWindow.hpp"
#include "Windowing/StateMachineWindow.hpp"

#include <memory>

int Editor::openEditor() {
    // 1) Engine init (once)
    auto state = new EngineState();
    state->init();
    EngineState::state = state;

    LuaRegistry::SetupLua(EngineState::state->luaEngine->state(), EngineState::state->currentActiveProjectDirectory);

    // 2) Create windows
    std::vector<std::unique_ptr<GameWindow>> windows;
    windows.emplace_back(std::make_unique<EditorWindow>());
    windows.back()->init();

    // Create StateMachine window sharing GL objects with the editor window
    GLFWwindow* share = windows[0]->window();
    windows.emplace_back(std::make_unique<StateMachineWindow>(share));
    windows.back()->init();

    // 3) Drive all windows from one loop
    while (!windows.empty())
    {
        // Tick all windows
        for (auto& w : windows)
            w->tick();

        // Shutdown + remove closed windows
        windows.erase(
            std::remove_if(windows.begin(), windows.end(), [](std::unique_ptr<GameWindow>& w)
            {
                if (w->shouldClose())
                {
                    w->shutdown();
                    return true;
                }
                return false;
            }),
            windows.end());

        glfwPollEvents();
    }

    // Note: GLFW is initialized inside Shared::initAWindow(). We terminate it here once.
    glfwTerminate();
    return EXIT_SUCCESS;
}
