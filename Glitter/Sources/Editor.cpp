//
// Created by subha on 20-12-2025.
//

#include "Editor.hpp"
#include "Helpers/glitter.hpp"

// System Headers
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdlib>
#include <windows.h>
#include "Controls/Input.hpp"
#include "3DModel/model.hpp"
#include <vector>
#include <algorithm>
#include "UI/outliner.hpp"
#include "../Headers/UI/AssetBrowser/AssetBrowser.hpp"
#include <EngineState.hpp>
#include "Level/Level.hpp"
#include <Controls/PlayerController.hpp>
#include "Windowing/EditorWindow.hpp"
#include "Windowing/StateMachineWindow.hpp"
#include <memory>

namespace
{
    template<typename TWindow, typename... TArgs>
    void spawnOrFocusWindowOnRequest(
        std::vector<std::unique_ptr<GameWindow>>& windows,
        bool& requestToOpenWindow,
        GLFWwindow*& outGlfwHandle,
        TArgs&&... args)
    {
        if (!requestToOpenWindow)
            return;
        requestToOpenWindow = false;

        // If an instance is already alive, just show/focus it.
        if (outGlfwHandle && !glfwWindowShouldClose(outGlfwHandle))
        {
            glfwShowWindow(outGlfwHandle);
            glfwFocusWindow(outGlfwHandle);
            return;
        }

        windows.emplace_back(std::make_unique<TWindow>(std::forward<TArgs>(args)...));
        windows.back()->init();

        // `init()` implementations are expected to assign their GLFWwindow* into EngineState
        // (e.g. EngineState::state->mStatemachineWindow).
        if (outGlfwHandle)
        {
            glfwShowWindow(outGlfwHandle);
            glfwFocusWindow(outGlfwHandle);
        }
    }
}

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

    // Keep a typed pointer for request handling (we only create one EditorWindow).
    auto* editorWindow = static_cast<EditorWindow*>(windows.front().get());

    // Windows are spawned on-demand (e.g. StateMachineWindow is created when requested from the UI).

    // 3) Drive all windows from one loop
    GameWindow* activeWindow = nullptr;
    while (!windows.empty())
    {
        // Spawn on-demand tool windows requested from UI (handled by Editor.cpp, not EngineState).
        if (EngineState::state && editorWindow)
        {
            spawnOrFocusWindowOnRequest<StateMachineWindow>(
                windows,
                editorWindow->windowRequests().openStateMachineWindow,
                EngineState::state->mStatemachineWindow,
                nullptr);
        }

        // Pick the actively focused window.
        GameWindow* focused = nullptr;
        for (auto& w : windows)
        {
            if (w->window() && glfwGetWindowAttrib(w->window(), GLFW_FOCUSED))
            {
                focused = w.get();
                break;
            }
        }

        if (!focused)
            focused = activeWindow ? activeWindow : (windows.empty() ? nullptr : windows.front().get());

        // Switch GL + UI contexts ONLY when the active window changes.
        if (focused && focused != activeWindow)
        {
            activeWindow = focused;
            glfwMakeContextCurrent(activeWindow->window());

            // Each window owns its own ImGui + ImNodes context.
            // The Input callbacks use WindowInputUserData to set the correct context still.
            if (auto* ud = static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(activeWindow->window())))
            {
                if (ud->imguiCtx) ImGui::SetCurrentContext(ud->imguiCtx);
                if (ud->imnodesCtx) ImNodes::SetCurrentContext(ud->imnodesCtx);
            }
        }

        // Poll events once per loop (GLFW events are process-global).
        // We keep it here, but conceptually this belongs to `activeWindow`.
        glfwPollEvents();

        // Tick ONLY the active window.
        if (activeWindow)
            activeWindow->tick();

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
    }

    // Note: GLFW is initialized inside Shared::initAWindow(). We terminate it here once.
    glfwTerminate();
    return EXIT_SUCCESS;
}
