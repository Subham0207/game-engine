#pragma once
#include "Controls/Input.hpp"
#include <iostream>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include <ImGuizmo.h>
#include "GLFW/glfw3.h"

// Still needed for getUIState() used by gizmo hotkeys.
#include <EngineState.hpp>

#include "Event/InputContext.hpp"


namespace
{
    static WindowInputUserData* GetWindowInputUserData(GLFWwindow* window)
    {
        return static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(window));
    }

    static void EnsureImguiForWindow(WindowInputUserData* ud)
    {
        if (!ud) return;
        if (ud->imguiCtx) ImGui::SetCurrentContext(ud->imguiCtx);
        if (ud->imnodesCtx) ImNodes::SetCurrentContext(ud->imnodesCtx);
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto* ud = GetWindowInputUserData(window);
        if (!ud) return;
        EnsureImguiForWindow(ud);
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }

    static void char_callback(GLFWwindow* window, unsigned int c)
    {
        auto* ud = GetWindowInputUserData(window);
        if (!ud) return;
        EnsureImguiForWindow(ud);
        ImGui_ImplGlfw_CharCallback(window, c);
    }

    static void window_focus_callback(GLFWwindow* window, int focused)
    {
        auto* ud = GetWindowInputUserData(window);
        if (!ud) return;
        EnsureImguiForWindow(ud);
        ImGui_ImplGlfw_WindowFocusCallback(window, focused);
    }
}

InputHandler::InputHandler(Camera* camera, GLFWwindow* window, float screenWidth, float screenHeight)
{
    m_Camera = camera;
    lastX = screenWidth;
    lastY = screenHeight;
    m_Window = window;
    mouseState = GLFW_CURSOR_DISABLED;
}

void InputHandler::handleInput(float deltaTime, InputContext& inputCtx, bool isPlay)
{
    if (glfwGetKey(m_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        if (!controlKeyPressed)
        {
            controlKeyPressed = true;  // Set the flag to indicate the key was processed
            if (mouseState == GLFW_CURSOR_NORMAL)
            {
                mouseState = GLFW_CURSOR_DISABLED;
            }
            else
            {
                mouseState = GLFW_CURSOR_NORMAL;
            }

            glfwSetInputMode(m_Window, GLFW_CURSOR, mouseState);
        }
    }
    else
    {
        controlKeyPressed = false;
    }

    if(!isPlay)
    {
        handleEditorInput(deltaTime, inputCtx);
    }
    else
    {
        handlePlay();
    }
}

void InputHandler::handleEditorInput(float deltaTime,InputContext& inputCtx)
{
    // IMPORTANT: Store both the InputHandler instance and InputContext per window.
    // This avoids global/static input state when multiple GLFW windows are open.
    auto* ud = GetWindowInputUserData(m_Window);
    if (!ud)
    {
        ud = new WindowInputUserData();
        glfwSetWindowUserPointer(m_Window, ud);
    }
    ud->handler = this;
    ud->ctx = &inputCtx;

    // Do not set imguiCtx/imnodesCtx here.
    // That is owned by the windowing layer (EditorWindow/StateMachineWindow) and should
    // be set once those contexts are created.

    // Ensure WantCaptureKeyboard/Mouse refer to the correct window's ImGui context.
    EnsureImguiForWindow(ud);

    handleBasicMovement(deltaTime);

    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);

    // Forward keyboard/text/focus events to ImGui (install_callbacks=false)
    glfwSetKeyCallback(m_Window, key_callback);
    glfwSetCharCallback(m_Window, char_callback);
    glfwSetWindowFocusCallback(m_Window, window_focus_callback);

}

void InputHandler::handlePlay()
{
    //TODO: remove this after checking usages...
}

bool InputHandler::isKeyPressed(int key) const
{
    return glfwGetKey(m_Window, key) == GLFW_PRESS;
}

void InputHandler::handleBasicMovement(float deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        return;

    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_Window, true);
    }

    const float cameraSpeed = movementSpeed * deltaTime; // per-window/per-handler
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        m_Camera->cameraPos += cameraSpeed * m_Camera->cameraFront;
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        m_Camera->cameraPos -= cameraSpeed * m_Camera->cameraFront;
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        m_Camera->cameraPos -= glm::normalize(glm::cross(m_Camera->cameraFront, m_Camera->cameraUp)) * cameraSpeed;
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        m_Camera->cameraPos += glm::normalize(glm::cross(m_Camera->cameraFront, m_Camera->cameraUp)) * cameraSpeed;
}

void InputHandler::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto* ud = GetWindowInputUserData(window);
    if (!ud || !ud->handler)
        return;
    auto* handler = ud->handler;

    EnsureImguiForWindow(ud);

    // First forward mouse move to ImGui (we use install_callbacks=false)
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    // If cursor is enabled, prefer UI interaction only.
    if (handler->mouseState != GLFW_CURSOR_DISABLED)
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return;
    }

    InputContext* ctx = ud->ctx;
    if (!ctx || !ctx->queue) return;

    //Once we know imgui is not processing that input; process the input.
    if (handler->firstMouse) // initially set to true
    {
        handler->lastX = xpos;
        handler->lastY = ypos;
        handler->firstMouse = false;
    }

    handler->xOffset = xpos - handler->lastX;
    handler->yOffset = handler->lastY - ypos; // reversed since y-coordinates range from bottom to top
    handler->lastX = xpos;
    handler->lastY = ypos;

    ctx->queue->push<MouseMoveEvent>(handler->getXOffset(), handler->getYOffset(), handler->mouseState);
}

void InputHandler::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto* ud = GetWindowInputUserData(window);
    if (!ud || !ud->handler)
        return;
    auto* handler = ud->handler;

    EnsureImguiForWindow(ud);

    ImGuiIO& io = ImGui::GetIO();

    // 1. Let ImGui have first dibs
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    // 2. If ImGui wants the mouse, stop here
    if (io.WantCaptureMouse) {
        handler->leftClickPressed = false;
        return;
    }

    // 3. If we clicked the "background" (game world), tell ImGui to drop focus
    if (action == GLFW_PRESS) {
        ImGui::SetWindowFocus(nullptr);
    }

    // When the mouse was clicked at IMGUI released did not help so setting mouseClickPressedTo false
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    handler->leftClickPressed = true;
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    handler->leftClickPressed = false;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    handler->rightClickPressed = true;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    handler->rightClickPressed = false;
}

void InputHandler::handleTransformGizmo(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        getUIState().whichTransformActive = ImGuizmo::OPERATION::TRANSLATE;
    }
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        getUIState().whichTransformActive = ImGuizmo::OPERATION::ROTATE;
    }
    if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        getUIState().whichTransformActive = ImGuizmo::OPERATION::SCALE;
    }
}
void InputHandler::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto* ud = GetWindowInputUserData(window);
    if (!ud || !ud->handler)
        return;
    auto* handler = ud->handler;

    EnsureImguiForWindow(ud);

    // Forward to ImGui first (install_callbacks=false)
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    if(ImGui::GetIO().WantCaptureMouse)
        return;
    handler->m_Camera->fov -= (float)yoffset;
    if (handler->m_Camera->fov < 1.0f)
        handler->m_Camera->fov = 1.0f;
    if (handler->m_Camera->fov > 90.0f)
        handler->m_Camera->fov = 90.0f;
}