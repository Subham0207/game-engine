#include "Windowing/GameWindow.hpp"

#include <EngineState.hpp>

#include "Camera/FlyCam.hpp"
#include "glad/glad.h"
#include "Helpers/Shared.hpp"

void GameWindow::tick()
{
    if (shouldClose())
        return;

    const float currentFrame = static_cast<float>(glfwGetTime());
    float dt = currentFrame - mLastFrame;
    mLastFrame = currentFrame;

    // Clamp to avoid huge physics steps when the window was unfocused / paused.
    if (dt < 0.0f) dt = 0.0f;
    if (dt > mMaxDeltaTime) dt = mMaxDeltaTime;

    mDeltaTime = dt;

    // Keep legacy global for systems still reading EngineState::state->deltaTime.
    if (EngineState::state)
        EngineState::state->deltaTime = mDeltaTime;

    // Update cached screen size (pixels) for this window.
    updateScreenSize();

    //Clear the screen and dispatch all events...
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mQueue)
    {
        mQueue->drain([&](const Event& e)
        {
            mBus->dispatch(e);
        });
    }

    onBeginFrame();
    tickImpl();
}

void GameWindow::initWindowAndBackends(
    const bool isToolWindow,
    const bool isMouseDisabled,
    const std::string& windowTitle,
    const bool initPhysics
    )
{
    mQueue = std::make_unique<EventQueue>();
    mBus = std::make_unique<EventBus>();
    mInputCtx = std::make_unique<InputContext>();
    mInputCtx->queue = mQueue.get();

    // Create a default camera for this window.
    // Windows that want to use EngineState's editorCamera can overwrite this pointer.
    if (!mCamera)
        mCamera = std::make_unique<FlyCam>("WindowCamera");

    mIsToolWindow = isToolWindow;
    mIsMouseDisabled = isMouseDisabled;
    mWindow = Shared::initAWindow(
        mIsVsyncOn,
        mIsToolWindow,
        windowTitle,
        mIsMouseDisabled
        );

    Shared::initGpuLogger();

    initUIBackends();

    if (initPhysics)
        getPhysicsSystem().Init();

    //Note the engine only registers mouse movment for default FlyCam used by Editor.
    //Any other camera created by user can have different movement logic to drive it.
    //For Example spring arm moves the camera. So user can subscribe to MouseMovement and build any custom logic.
    mBus->subscribe<MouseMoveEvent>([&](const MouseMoveEvent& e)
    {
        mCamera->onMouseMove(e);
    });
}

void GameWindow::initUIBackends()
{
    mImguiContext = Shared::createImguiContext();
    mImNodesContext = ImNodes::CreateContext();

    setImguiCurrent();
    ImNodes::SetCurrentContext(mImNodesContext);
    Shared::initImguiBackendForWindow(mWindow);

    // Attach ImGui contexts for per-window input routing.
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
}

