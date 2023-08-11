#include "Input.hpp"

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
    glfwSetCursorPosCallback(m_Window, mouse_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
}

void InputHandler::handleBasicMovement(float deltaTime)
{
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

void InputHandler::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    currentInputHandler->m_Camera->fov -= (float)yoffset;
    if (currentInputHandler->m_Camera->fov < 1.0f)
        currentInputHandler->m_Camera->fov = 1.0f;
    if (currentInputHandler->m_Camera->fov > 45.0f)
        currentInputHandler->m_Camera->fov = 45.0f;
}