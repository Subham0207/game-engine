#include "Controls/Input.hpp"
#include <iostream>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"

InputHandler* InputHandler::currentInputHandler = nullptr;

InputHandler::InputHandler(Camera* camera, GLFWwindow* window, float screenWidth, float screenHeight)
{
    m_Camera = camera;
    lastX = screenWidth;
    lastY = screenHeight;
    m_Window = window;
}

void InputHandler::handleInput(float deltaTime)
{
    handleBasicMovement(deltaTime);
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
        controlKeyPressed = false;  // Reset the flag when the key is released
    }

    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);

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
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)xpos, (float)ypos);
    if (io.WantCaptureMouse)
    return;

    //Once we know imgui is not processing that input; process the input.
    if (currentInputHandler->firstMouse) // initially set to true
    {
        currentInputHandler->lastX = xpos;
        currentInputHandler->lastY = ypos;
        currentInputHandler->firstMouse = false;
    }

    float xoffset = xpos - currentInputHandler->lastX;
    float yoffset = currentInputHandler->lastY - ypos; // reversed since y-coordinates range from bottom to top
    currentInputHandler->lastX = xpos;
    currentInputHandler->lastY = ypos;

    if(currentInputHandler->mouseState == GLFW_CURSOR_NORMAL)
        return;

    const float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    currentInputHandler->yaw += xoffset;
    currentInputHandler->pitch += yoffset;

    if (currentInputHandler->pitch > 89.0f)
        currentInputHandler->pitch = 89.0f;
    if (currentInputHandler->pitch < -89.0f)
        currentInputHandler->pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(currentInputHandler->yaw)) * cos(glm::radians(currentInputHandler->pitch));
    direction.y = sin(glm::radians(currentInputHandler->pitch));
    direction.z = sin(glm::radians(currentInputHandler->yaw)) * cos(glm::radians(currentInputHandler->pitch));
    currentInputHandler->m_Camera->cameraFront = glm::normalize(direction);
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
}

void InputHandler::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(ImGui::GetIO().WantCaptureMouse)
    {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        return;
    }
    currentInputHandler->m_Camera->fov -= (float)yoffset;
    if (currentInputHandler->m_Camera->fov < 1.0f)
        currentInputHandler->m_Camera->fov = 1.0f;
    if (currentInputHandler->m_Camera->fov > 45.0f)
        currentInputHandler->m_Camera->fov = 45.0f;
}