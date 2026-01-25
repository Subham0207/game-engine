#pragma once
#include "Camera/Camera.hpp"
#include "Event/InputContext.hpp"

class GLFWWindow;
class InputHandler
{
public:
	InputHandler(Camera* camera, GLFWwindow* m_Window,float screenWidth, float screenHeight);
	void handleInput(float deltaTime, InputContext& inputCtx);
	static InputHandler* currentInputHandler;
	unsigned int mouseState;
	float lastX,lastY;
	Camera* m_Camera;
	bool leftClickPressed = false;
	bool rightClickPressed = false;
	float getXOffset(){return xOffset;}
	float getYOffset(){return yOffset;}

	[[nodiscard]] bool isKeyPressed(int key) const;
private:
	void handleBasicMovement(float deltaTime);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	static void handleTransformGizmo(GLFWwindow* window);

	void handleEditorInput(float deltaTime, InputContext& inputCtx);
	void handlePlay();
	bool firstMouse = true;
	float yaw = -90.0f;
	float pitch = 0.0f;

	float xOffset = 0.0f;
	float yOffset = 0.0f;

	bool controlKeyPressed = false;

	GLFWwindow* m_Window;

};

