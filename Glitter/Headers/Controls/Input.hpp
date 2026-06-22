#pragma once
#include "imgui_internal.h"
#include "imnodes_internal.h"
#include "Camera/Camera.hpp"
#include "Event/InputContext.hpp"

namespace Rml
{
	class Context;
}

class GLFWWindow;
class InputHandler
{
public:
	InputHandler(Camera* camera, GLFWwindow* m_Window,float screenWidth, float screenHeight);
	void handleInput(float deltaTime, InputContext& inputCtx, bool isPlay);

	// Movement speed used by editor-style camera controls (WASD).
	// Windowing layer should set this as needed; defaults to a reasonable value.
	float movementSpeed = 30.f;
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

// Per-window callback payload stored as GLFW window user pointer when using InputHandler.
// Windowing layer may also store the ImGui/ImNodes contexts here so callbacks can set
// correct current context before forwarding events.
struct WindowInputUserData
{
	using KeyCallback = bool (*)(GLFWwindow* window, int key, int scancode, int action, int mods);
	using CharCallback = bool (*)(GLFWwindow* window, unsigned int c);
	using CursorEnterCallback = bool (*)(GLFWwindow* window, int entered);
	using CursorPosCallback = bool (*)(GLFWwindow* window, double xpos, double ypos);
	using MouseButtonCallback = bool (*)(GLFWwindow* window, int button, int action, int mods);
	using ScrollCallback = bool (*)(GLFWwindow* window, double xoffset, double yoffset);
	using FramebufferSizeCallback = void (*)(GLFWwindow* window, int width, int height);

	InputHandler* handler = nullptr;
	InputContext* ctx = nullptr;
	ImGuiContext* imguiCtx = nullptr;
	ImNodesContext* imnodesCtx = nullptr;
	Rml::Context* rmlContext = nullptr;
	int rmlModifierState = 0;
	KeyCallback onKey = nullptr;
	CharCallback onChar = nullptr;
	CursorEnterCallback onCursorEnter = nullptr;
	CursorPosCallback onCursorPos = nullptr;
	MouseButtonCallback onMouseButton = nullptr;
	ScrollCallback onScroll = nullptr;
	FramebufferSizeCallback onFramebufferSize = nullptr;
};

