#pragma once
#include <GLFW/glfw3.h>
#include "Camera.h"

class InputHandler
{
public:
	InputHandler(Camera* camera, GLFWwindow* m_Window,float screenWidth, float screenHeight);
	void handleInput(float deltaTime);
	static InputHandler* currentInputHandler;
private:
	void handleBasicMovement(float deltaTime);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	Camera* m_Camera;
	bool firstMouse = true;
	float lastX,lastY;
	float yaw = -90.0f;
	float pitch = 0.0f;

	GLFWwindow* m_Window;
};

