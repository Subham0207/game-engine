#pragma once
#include "GLFW/glfw3.h"
#include "Camera/Camera.hpp"

class InputHandler
{
public:
	InputHandler(Camera* camera, GLFWwindow* m_Window,float screenWidth, float screenHeight);
	void handleInput(float deltaTime);
	static InputHandler* currentInputHandler;
	unsigned int mouseState = GLFW_CURSOR_DISABLED;
	float lastX,lastY;
	Camera* m_Camera;
	bool leftClickPressed = false;
private:
	void handleBasicMovement(float deltaTime);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	static void handleTransformGizmo(GLFWwindow* window);

	void handleEditorInput(float deltaTime);
	void handlePlay();

	bool firstMouse = true;
	float yaw = -90.0f;
	float pitch = 0.0f;

	bool controlKeyPressed = false;

	GLFWwindow* m_Window;

};

