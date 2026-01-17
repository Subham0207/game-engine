#pragma once
#include "Controls/Input.hpp"
#include <iostream>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include <EngineState.hpp>
#include <ImGuizmo.h>

#include "Event/InputContext.hpp"

InputHandler* InputHandler::currentInputHandler = nullptr;

InputHandler::InputHandler(Camera* camera, GLFWwindow* window, float screenWidth, float screenHeight)
{
    m_Camera = camera;
    lastX = screenWidth;
    lastY = screenHeight;
    m_Window = window;
}

void InputHandler::handleInput(float deltaTime, InputContext& inputCtx)
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

    if(!EngineState::state->isPlay)
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
    handleBasicMovement(deltaTime);
    glfwSetWindowUserPointer(m_Window, &inputCtx);
    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);

}

void InputHandler::handlePlay()
{
    //Get the first PlayerController or activePlayer controller
    //A Player controller for now will just pass these button presses as boolean. This is for abstraction purpose
    //That means the state machine would be called on every frame but based on if the player controller has a input for it it would respond
    //We can place the state machine update in character update for now
    auto id = EngineState::state->activePlayerControllerId;
    Controls::PlayerController* playerController = nullptr;
    if(EngineState::state->playerControllers.size() > 0)
    playerController = EngineState::state->playerControllers.at(id);

    if(!playerController)
    return;

    glm::vec3 directionVector(0.0f);
    //W to move
    playerController->isForwardPressed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        directionVector += glm::vec3(0,0,1);
        playerController->isForwardPressed = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        directionVector += glm::vec3(1,0,0);
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        directionVector += glm::vec3(-1,0,0);
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        directionVector += glm::vec3(0,0,-1);
    if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS && playerController->grounded)
        playerController->isJumping = true;
    if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
    {
        playerController->dodgeStart = true;
        directionVector = glm::vec3(0.0f);
    }

    playerController->isAiming = rightClickPressed;

    if (glm::length(directionVector) > 0.00001f) {
        playerController->setMovement(glm::normalize(directionVector));
    }
    else
    {
        playerController->setMovement(glm::vec3(0.0f,0.0f,0.0f));
    }
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

    const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
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
    //First process inputs for imgui
    if(currentInputHandler->mouseState != GLFW_CURSOR_DISABLED)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2((float)xpos, (float)ypos);
        if (io.WantCaptureMouse)
        return;
    }

    auto* ctx = static_cast<InputContext*>(glfwGetWindowUserPointer(window));
    if (!ctx || !ctx->queue)return;

    //Once we know imgui is not processing that input; process the input.
    if (currentInputHandler->firstMouse) // initially set to true
    {
        currentInputHandler->lastX = xpos;
        currentInputHandler->lastY = ypos;
        currentInputHandler->firstMouse = false;
    }

    currentInputHandler->xOffset = xpos - currentInputHandler->lastX;
    currentInputHandler->yOffset = currentInputHandler->lastY - ypos; // reversed since y-coordinates range from bottom to top
    currentInputHandler->lastX = xpos;
    currentInputHandler->lastY = ypos;

    ctx->queue->push<MouseMoveEvent>(currentInputHandler->getXOffset(), currentInputHandler->getYOffset(), currentInputHandler->mouseState);
}

void InputHandler::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        InputHandler::currentInputHandler->leftClickPressed = false;
        return; // Optional: return here if you don't want to process clicks further when ImGui uses them
    }

    ImGui::SetWindowFocus(false);

    // When the mouse was clicked at IMGUI released did not help so setting mouseClickPressedTo false
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    InputHandler::currentInputHandler->leftClickPressed = true;
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    InputHandler::currentInputHandler->leftClickPressed = false;

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    InputHandler::currentInputHandler->rightClickPressed = true;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    InputHandler::currentInputHandler->rightClickPressed = false;
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
    if(ImGui::GetIO().WantCaptureMouse)
    {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        return;
    }
    currentInputHandler->m_Camera->fov -= (float)yoffset;
    if (currentInputHandler->m_Camera->fov < 1.0f)
        currentInputHandler->m_Camera->fov = 1.0f;
    if (currentInputHandler->m_Camera->fov > 90.0f)
        currentInputHandler->m_Camera->fov = 90.0f;
}