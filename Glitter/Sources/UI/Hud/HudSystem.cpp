#include "UI/Hud/HudSystem.hpp"

#include "Controls/Input.hpp"

#include <glad/glad.h>
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi_Platform_GLFW.h>
#include <RmlUi_Renderer_GL3.h>

#include <iostream>

namespace
{
    bool gRmlInitialized = false;

    WindowInputUserData* getWindowUserData(GLFWwindow* window)
    {
        return static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(window));
    }

    bool hudKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        return !RmlGLFW::ProcessKeyCallback(ud->rmlContext, key, action, mods);
    }

    bool hudCharCallback(GLFWwindow* window, unsigned int c)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        return !RmlGLFW::ProcessCharCallback(ud->rmlContext, c);
    }

    bool hudCursorEnterCallback(GLFWwindow* window, int entered)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        return !RmlGLFW::ProcessCursorEnterCallback(ud->rmlContext, entered);
    }

    bool hudCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        return !RmlGLFW::ProcessCursorPosCallback(ud->rmlContext, window, xpos, ypos, ud->rmlModifierState);
    }

    bool hudMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        ud->rmlModifierState = mods;
        return !RmlGLFW::ProcessMouseButtonCallback(ud->rmlContext, button, action, mods);
    }

    bool hudScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        return !RmlGLFW::ProcessScrollCallback(ud->rmlContext, yoffset, ud->rmlModifierState);
    }

    void hudFramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return;

        RmlGLFW::ProcessFramebufferSizeCallback(ud->rmlContext, width, height);
    }
}

namespace UI::Hud
{
    HudSystem::~HudSystem()
    {
        shutdown();
    }

    bool HudSystem::init(GLFWwindow* window, int initialWidth, int initialHeight, const std::filesystem::path& documentPath)
    {
        if (mInitialized || !window)
            return false;

        if (gRmlInitialized)
            return false;

        mWindow = window;

        Rml::String rendererMessage;
        if (!RmlGL3::Initialize(&rendererMessage))
        {
            std::cerr << "[HUD] Failed to initialize RmlUi OpenGL3 renderer" << std::endl;
            return false;
        }

        mSystemInterface = new SystemInterface_GLFW();
        mSystemInterface->SetWindow(mWindow);

        mRenderInterface = new RenderInterface_GL3();
        if (!mRenderInterface || !(*mRenderInterface))
        {
            std::cerr << "[HUD] Failed to construct RmlUi GL3 render interface" << std::endl;
            RmlGL3::Shutdown();
            delete mRenderInterface;
            mRenderInterface = nullptr;
            delete mSystemInterface;
            mSystemInterface = nullptr;
            return false;
        }

        if (initialWidth <= 0 || initialHeight <= 0)
            glfwGetFramebufferSize(mWindow, &initialWidth, &initialHeight);

        mRenderInterface->SetViewport(initialWidth, initialHeight);

        Rml::SetSystemInterface(mSystemInterface);
        Rml::SetRenderInterface(mRenderInterface);

        // This workspace currently builds RmlUi with RMLUI_FONT_ENGINE=none.
        // Provide the default no-op font engine so Rml::Initialise() can still succeed.
        if (!Rml::GetFontEngineInterface())
        {
            mFallbackFontEngine = new Rml::FontEngineInterface();
            Rml::SetFontEngineInterface(mFallbackFontEngine);
        }

        if (!Rml::Initialise())
        {
            std::cerr << "[HUD] Rml::Initialise failed (check font engine and render interface setup)" << std::endl;
            RmlGL3::Shutdown();
            delete mRenderInterface;
            mRenderInterface = nullptr;
            delete mSystemInterface;
            mSystemInterface = nullptr;
            delete mFallbackFontEngine;
            mFallbackFontEngine = nullptr;
            return false;
        }

        mContext = Rml::CreateContext("EditorHUD", Rml::Vector2i(initialWidth, initialHeight));
        if (!mContext)
        {
            std::cerr << "[HUD] Failed to create RmlUi context" << std::endl;
            Rml::Shutdown();
            RmlGL3::Shutdown();
            delete mRenderInterface;
            mRenderInterface = nullptr;
            delete mSystemInterface;
            mSystemInterface = nullptr;
            return false;
        }

        const auto documentPathString = documentPath.generic_string();
        std::cout << "[HUD] Loading document: " << documentPathString << std::endl;
        mDocument = mContext->LoadDocument(documentPathString);
        if (!mDocument)
            std::cerr << "[HUD] Failed to load HUD document: " << documentPathString << std::endl;
        else
        {
            std::cout << "[HUD] Document loaded successfully" << std::endl;
            mDocument->Show();

            // Keep HUD visible even if project RCSS isn't resolved by forcing core styles at runtime.
            if (auto* healthFrame = mDocument->GetElementById("health-frame"))
            {
                healthFrame->SetProperty("position", "absolute");
                healthFrame->SetProperty("left", "24px");
                healthFrame->SetProperty("bottom", "52px");
                healthFrame->SetProperty("width", "300px");
                healthFrame->SetProperty("height", "20px");
                healthFrame->SetProperty("background-color", "#2a2a2a");
            }

            if (auto* healthFill = mDocument->GetElementById("health-fill"))
            {
                healthFill->SetProperty("width", "100%");
                healthFill->SetProperty("height", "100%");
                healthFill->SetProperty("background-color", "#c0392b");
            }

            if (auto* staminaFrame = mDocument->GetElementById("stamina-frame"))
            {
                staminaFrame->SetProperty("position", "absolute");
                staminaFrame->SetProperty("left", "24px");
                staminaFrame->SetProperty("bottom", "24px");
                staminaFrame->SetProperty("width", "300px");
                staminaFrame->SetProperty("height", "20px");
                staminaFrame->SetProperty("background-color", "#2a2a2a");
            }

            if (auto* staminaFill = mDocument->GetElementById("stamina-fill"))
            {
                staminaFill->SetProperty("width", "75%");
                staminaFill->SetProperty("height", "100%");
                staminaFill->SetProperty("background-color", "#27ae60");
            }

            if (auto* crosshair = mDocument->GetElementById("crosshair"))
            {
                crosshair->SetProperty("position", "absolute");
                crosshair->SetProperty("left", "50%");
                crosshair->SetProperty("top", "50%");
                crosshair->SetProperty("margin-left", "-2px");
                crosshair->SetProperty("margin-top", "-2px");
                crosshair->SetProperty("width", "4px");
                crosshair->SetProperty("height", "4px");
                crosshair->SetProperty("background-color", "#f0f0f0");
            }
        }

        auto* ud = static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(mWindow));
        if (!ud)
        {
            ud = new WindowInputUserData();
            glfwSetWindowUserPointer(mWindow, ud);
        }
        ud->rmlContext = mContext;
        ud->onKey = hudKeyCallback;
        ud->onChar = hudCharCallback;
        ud->onCursorEnter = hudCursorEnterCallback;
        ud->onCursorPos = hudCursorPosCallback;
        ud->onMouseButton = hudMouseButtonCallback;
        ud->onScroll = hudScrollCallback;
        ud->onFramebufferSize = hudFramebufferSizeCallback;

        gRmlInitialized = true;
        mInitialized = true;
        return true;
    }

    void HudSystem::tick(int viewportWidth, int viewportHeight)
    {
        if (!mInitialized || !mContext || !mRenderInterface)
            return;

        if (!mLoggedFramebufferState)
        {
            GLint drawFramebuffer = 0;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
            std::cout << "[HUD] GL_DRAW_FRAMEBUFFER_BINDING before HUD render: " << drawFramebuffer << std::endl;
            mLoggedFramebufferState = true;
        }

        if (viewportWidth > 0 && viewportHeight > 0)
        {
            mRenderInterface->SetViewport(viewportWidth, viewportHeight);
            mContext->SetDimensions(Rml::Vector2i(viewportWidth, viewportHeight));
        }

        mContext->Update();
        mRenderInterface->BeginFrame();
        mContext->Render();
        mRenderInterface->EndFrame();
    }

    void HudSystem::shutdown()
    {
        if (!mInitialized)
            return;

        if (mWindow)
        {
            auto* ud = static_cast<WindowInputUserData*>(glfwGetWindowUserPointer(mWindow));
            if (ud)
            {
                ud->rmlContext = nullptr;
                ud->onKey = nullptr;
                ud->onChar = nullptr;
                ud->onCursorEnter = nullptr;
                ud->onCursorPos = nullptr;
                ud->onMouseButton = nullptr;
                ud->onScroll = nullptr;
                ud->onFramebufferSize = nullptr;
            }
        }

        mDocument = nullptr;
        mContext = nullptr;

        Rml::Shutdown();
        RmlGL3::Shutdown();

        delete mFallbackFontEngine;
        mFallbackFontEngine = nullptr;

        delete mRenderInterface;
        mRenderInterface = nullptr;

        delete mSystemInterface;
        mSystemInterface = nullptr;
        mWindow = nullptr;

        mInitialized = false;
        gRmlInitialized = false;
        mLoggedFramebufferState = false;
    }
}




