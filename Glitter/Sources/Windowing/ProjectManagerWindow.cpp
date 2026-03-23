#include "Windowing/ProjectManagerWindow.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Helpers/Shared.hpp>

#include <UI/ProjectManager.hpp>

#include "Controls/Input.hpp"

#include "Helpers/GetExecutablePath.hpp"

#include <cstdlib>

ProjectManagerWindow::ProjectManagerWindow()
{
	mPath = GetExecutablePath::getExecutableDir();
	mProjectManagerUI = std::make_unique<ProjectManagerUI::ProjectManager>(mPath.string(), mPath.string());
}

ProjectManagerWindow::~ProjectManagerWindow() = default;

void ProjectManagerWindow::init()
{
	initWindowAndBackends(true, false, "Project Manager", false);

	if (!mWindow)
		return;

	// Create isolated contexts/backends for this window.
	mImguiContext = Shared::createImguiContext();
	mImNodesContext = Shared::createImNodesContext();
	setImguiCurrent();
	ImNodes::SetCurrentContext(mImNodesContext);
	Shared::initImguiBackendForWindow(mWindow);

	// Attach contexts for per-window callback routing.
	{
		auto* ud = static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(mWindow));
		if (!ud)
		{
			ud = new WindowInputUserData();
			glfwSetWindowUserPointer(mWindow, ud);
		}
		ud->imguiCtx = mImguiContext;
		ud->imnodesCtx = mImNodesContext;
	}


	//TODO: Below can be moved to using mScreenWidth, mScreenHeight.
	// IMPORTANT:
	// ImGui backend was initialized with install_callbacks=false.
	// Therefore the engine must install GLFW callbacks and forward events to ImGui.
	// InputHandler::handleInput() performs this setup.
	int fbW = 0, fbH = 0;
	glfwGetFramebufferSize(mWindow, &fbW, &fbH);
	if (fbW <= 0) fbW = 1280;
	if (fbH <= 0) fbH = 720;

	// Use the per-window default camera from GameWindow.
	// This avoids requiring EngineState for ProjectManager exe.
	mInputHandler = std::make_unique<InputHandler>(mCamera.get(), mWindow, (float)fbW, (float)fbH);
	mInputHandler->movementSpeed = 0.0f;
	mInputHandler->handleInput(0.0f, *mInputCtx, false);
}

void ProjectManagerWindow::tickImpl()
{
	if (!mWindow)
		return;

	// Keep per-window callback routing updated (installs callbacks, updates WindowInputUserData)
	if (mInputHandler)
		mInputHandler->handleInput(mDeltaTime, *mInputCtx, false);

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (mProjectManagerUI)
		mProjectManagerUI->draw();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(mWindow);
}

void ProjectManagerWindow::shutdown()
{
	if (!mWindow)
		return;

	makeCurrent();
	if (mImguiContext)
		ImGui::SetCurrentContext(mImguiContext);
	if (mImNodesContext)
		ImNodes::SetCurrentContext(mImNodesContext);

	Shared::shutdownImguiBackendForWindow();
	if (mImNodesContext) { ImNodes::DestroyContext(mImNodesContext); mImNodesContext = nullptr; }
	if (mImguiContext) { ImGui::DestroyContext(mImguiContext); mImguiContext = nullptr; }

	glfwDestroyWindow(mWindow);
	mWindow = nullptr;
}

int ProjectManagerWindow::StartWindow()
{
	init();
	while (!shouldClose())
	{
		glfwPollEvents();
		tick();
	}
	shutdown();

	glfwTerminate();
	return EXIT_SUCCESS;
}






