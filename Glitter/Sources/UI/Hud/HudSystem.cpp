#include "UI/Hud/HudSystem.hpp"

#include "Controls/Input.hpp"
#include <EngineState.hpp>

#include <filesystem>

#include <glad/glad.h>
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi_Platform_GLFW.h>
#include <RmlUi_Renderer_GL3.h>

#include <iostream>

namespace fs = std::filesystem;

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
        if (!rendererMessage.empty())
            std::cout << "[HUD] RmlGL3 message: " << rendererMessage << std::endl;

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

        if (!Rml::Initialise())
        {
            std::cerr << "[HUD] Rml::Initialise failed (check font engine and render interface setup)" << std::endl;
            RmlGL3::Shutdown();
            delete mRenderInterface;
            mRenderInterface = nullptr;
            delete mSystemInterface;
            mSystemInterface = nullptr;
            return false;
        }

        // Prefer project fonts first; fallback to engine fonts. This verifies whether a real font engine is active.
        const fs::path projectRobotoDir = documentPath.parent_path().parent_path() / "Roboto";
        const fs::path engineRobotoDir = fs::path(EngineState::state->engineInstalledDirectory) / "EngineAssets" / "Roboto";

        const fs::path regularFontPath = fs::exists(projectRobotoDir / "Roboto-Regular.ttf")
            ? (projectRobotoDir / "Roboto-Regular.ttf")
            : (engineRobotoDir / "Roboto-Regular.ttf");
        const fs::path boldFontPath = fs::exists(projectRobotoDir / "Roboto-Bold.ttf")
            ? (projectRobotoDir / "Roboto-Bold.ttf")
            : (engineRobotoDir / "Roboto-Bold.ttf");

        const bool regularLoaded = Rml::LoadFontFace(regularFontPath.generic_string(), true);
        const bool boldLoaded = Rml::LoadFontFace(boldFontPath.generic_string(), false);
        std::cout << "[HUD] LoadFontFace regular=" << regularLoaded << " path=" << regularFontPath << std::endl;
        std::cout << "[HUD] LoadFontFace bold=" << boldLoaded << " path=" << boldFontPath << std::endl;

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
            auto* healthFrame = mDocument->GetElementById("health-frame");
            auto* healthFill = mDocument->GetElementById("health-fill");
            auto* staminaFrame = mDocument->GetElementById("stamina-frame");
            auto* staminaFill = mDocument->GetElementById("stamina-fill");
            auto* crosshair = mDocument->GetElementById("crosshair");

            std::cout << "[HUD] ids health-frame=" << (healthFrame ? 1 : 0)
                << " health-fill=" << (healthFill ? 1 : 0)
                << " stamina-frame=" << (staminaFrame ? 1 : 0)
                << " stamina-fill=" << (staminaFill ? 1 : 0)
                << " crosshair=" << (crosshair ? 1 : 0) << std::endl;

            if (healthFrame)
            {
                healthFrame->SetProperty("position", "absolute");
                healthFrame->SetProperty("left", "24px");
                healthFrame->SetProperty("bottom", "52px");
                healthFrame->SetProperty("width", "300px");
                healthFrame->SetProperty("height", "20px");
                healthFrame->SetProperty("background-color", "#2a2a2a");
            }

            if (healthFill)
            {
                healthFill->SetProperty("width", "100%");
                healthFill->SetProperty("height", "100%");
                healthFill->SetProperty("background-color", "#c0392b");
            }

            if (staminaFrame)
            {
                staminaFrame->SetProperty("position", "absolute");
                staminaFrame->SetProperty("left", "24px");
                staminaFrame->SetProperty("bottom", "24px");
                staminaFrame->SetProperty("width", "300px");
                staminaFrame->SetProperty("height", "20px");
                staminaFrame->SetProperty("background-color", "#2a2a2a");
            }

            if (staminaFill)
            {
                staminaFill->SetProperty("width", "75%");
                staminaFill->SetProperty("height", "100%");
                staminaFill->SetProperty("background-color", "#27ae60");
            }

            if (crosshair)
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

            // Guaranteed debug primitive from Rml DOM to verify geometry visibility.
            Rml::ElementPtr debugQuad = mDocument->CreateElement("div");
            if (debugQuad)
            {
                debugQuad->SetId("hud-debug-quad");
                debugQuad->SetProperty("position", "absolute");
                debugQuad->SetProperty("right", "24px");
                debugQuad->SetProperty("top", "24px");
                debugQuad->SetProperty("width", "32px");
                debugQuad->SetProperty("height", "32px");
                debugQuad->SetProperty("background-color", "#ffff00");
                debugQuad->SetProperty("z-index", "9999");
                mDocument->AppendChild(std::move(debugQuad));
                std::cout << "[HUD] Added debug quad element" << std::endl;
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

        // Start from a known-good state before entering the Rml renderer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, viewportWidth, viewportHeight);
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_SCISSOR_TEST); // ImGui can leave a restrictive scissor rect active.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        mRenderInterface->BeginFrame();
        mContext->Render();
        mRenderInterface->EndFrame();

        // Guaranteed top-right yellow probe (independent of Rml DOM) for visibility debugging.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GLboolean scissorWasEnabled = GL_FALSE;
        glGetBooleanv(GL_SCISSOR_TEST, &scissorWasEnabled);
        glEnable(GL_SCISSOR_TEST);
        const int probeSize = 64;
        const int probeX = viewportWidth > probeSize ? (viewportWidth - probeSize) : 0;
        const int probeY = viewportHeight > probeSize ? (viewportHeight - probeSize) : 0;
        glScissor(probeX, probeY, probeSize, probeSize);
        glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (!scissorWasEnabled)
            glDisable(GL_SCISSOR_TEST);
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




