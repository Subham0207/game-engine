#include "UI/Hud/HudSystem.hpp"

#include "Controls/Input.hpp"
#include "Event/EventBus.hpp"
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

        RmlGLFW::ProcessKeyCallback(ud->rmlContext, key, action, mods);
        return false;
    }

    bool hudCharCallback(GLFWwindow* window, unsigned int c)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        RmlGLFW::ProcessCharCallback(ud->rmlContext, c);
        return false;
    }

    bool hudCursorEnterCallback(GLFWwindow* window, int entered)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        RmlGLFW::ProcessCursorEnterCallback(ud->rmlContext, entered);
        return false;
    }

    bool hudCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        RmlGLFW::ProcessCursorPosCallback(ud->rmlContext, window, xpos, ypos, ud->rmlModifierState);
        return false;
    }

    bool hudMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        ud->rmlModifierState = mods;
        RmlGLFW::ProcessMouseButtonCallback(ud->rmlContext, button, action, mods);
        return false;
    }

    bool hudScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto* ud = getWindowUserData(window);
        if (!ud || !ud->rmlContext)
            return false;

        RmlGLFW::ProcessScrollCallback(ud->rmlContext, yoffset, ud->rmlModifierState);
        return false;
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

    bool HudSystem::init(GLFWwindow* window, int initialWidth, int initialHeight, EventBus* eventBus)
    {
        if (mInitialized || !window)
            return false;

        if (gRmlInitialized)
            return false;

        mWindow = window;
        mEventBus = eventBus;

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

        // Prefer project fonts first; fallback to engine fonts.
        const fs::path activeProjectDir = EngineState::state
            ? fs::path(EngineState::state->currentActiveProjectDirectory)
            : fs::path();
        const fs::path engineInstallDir = EngineState::state
            ? fs::path(EngineState::state->engineInstalledDirectory)
            : fs::path();

        const fs::path projectRobotoDir = activeProjectDir / "Assets" / "Roboto";
        const fs::path engineRobotoDir = engineInstallDir / "EngineAssets" / "Roboto";

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

        float dpiScaleX = 1.0f;
        float dpiScaleY = 1.0f;
        glfwGetWindowContentScale(mWindow, &dpiScaleX, &dpiScaleY);
        mContext->SetDensityIndependentPixelRatio(dpiScaleX);

        if (mEventBus)
        {
            mEventBus->subscribe<HUDUpdateEvent>([this](const HUDUpdateEvent& e)
            {
                onHUDUpdateEvent(e);
            });

            mEventBus->subscribe<ActivateHUDEvent>([this](const ActivateHUDEvent& e)
            {
                onActivateHUDEvent(e);
            });
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

    void HudSystem::discoverHudDocuments(const std::filesystem::path& hudDirectory)
    {
        if (hudDirectory.empty() || !fs::exists(hudDirectory) || !fs::is_directory(hudDirectory))
        {
            std::cerr << "[HUD] HUD directory missing: " << hudDirectory << std::endl;
            return;
        }

        int discoveredCount = 0;
        for (const auto& entry : fs::directory_iterator(hudDirectory))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".rml")
                continue;

            const fs::path rmlPath = entry.path();
            const std::string key = rmlPath.stem().string();
            const fs::path rcssPath = rmlPath.parent_path() / (key + ".rcss");

            registerHudDocument(key, rmlPath, rcssPath);
            ++discoveredCount;
        }

        std::cout << "[HUD] Discovered " << discoveredCount << " HUD document(s) in " << hudDirectory << std::endl;
    }

    void HudSystem::registerHudDocument(const std::string& key, const std::filesystem::path& rmlPath, const std::filesystem::path& rcssPath)
    {
        if (key.empty())
        {
            std::cerr << "[HUD] registerHudDocument skipped: empty key for path " << rmlPath << std::endl;
            return;
        }

        if (!rcssPath.empty() && !fs::exists(rcssPath))
        {
            std::cerr << "[HUD] No matching RCSS for key=" << key << " expected=" << rcssPath << std::endl;
        }

        mHudRegistry[key] = HudDefinition{rmlPath, rcssPath};
    }

    bool HudSystem::activateHud(const std::string& key)
    {
        if (!mContext)
            return false;

        if (mDocument)
        {
            mDocument->Close();
            mDocument = nullptr;
            mActiveHudKey.clear();
        }

        const auto it = mHudRegistry.find(key);
        if (it == mHudRegistry.end())
        {
            std::cerr << "[HUD] ActivateHUDEvent failed. HUD key not registered: " << key << std::endl;
            return false;
        }

        const auto& hudDef = it->second;
        if (hudDef.rmlPath.empty() || !fs::exists(hudDef.rmlPath))
        {
            std::cerr << "[HUD] ActivateHUDEvent failed. HUD rml missing for key " << key
                      << " path=" << hudDef.rmlPath << std::endl;
            return false;
        }

        const auto documentPathString = hudDef.rmlPath.generic_string();
        std::cout << "[HUD] Loading document: " << documentPathString << std::endl;
        mDocument = mContext->LoadDocument(documentPathString);
        if (!mDocument)
        {
            std::cerr << "[HUD] Failed to load HUD document: " << documentPathString << std::endl;
            return false;
        }

        mDocument->SetProperty("display", "block");
        mDocument->SetProperty("width", "100%");
        mDocument->SetProperty("height", "100%");
        mDocument->Show();
        mActiveHudKey = key;

        std::cout << "[HUD] Activated HUD: " << key
                  << " rml=" << hudDef.rmlPath
                  << " rcss=" << hudDef.rcssPath << std::endl;
        return true;
    }

    bool HudSystem::setElementStyleById(const std::string& id, const std::string& property, const std::string& value)
    {
        if (!mDocument)
            return false;

        auto* element = mDocument->GetElementById(id);
        if (!element)
            return false;

        element->SetProperty(property, value);
        return true;
    }

    bool HudSystem::setElementTextById(const std::string& id, const std::string& value)
    {
        if (!mDocument)
            return false;

        auto* element = mDocument->GetElementById(id);
        if (!element)
            return false;

        element->SetInnerRML(value);
        return true;
    }

    void HudSystem::onHUDUpdateEvent(const HUDUpdateEvent& e)
    {
        bool updated = false;
        switch (e.operation)
        {
        case HUDUpdateOperation::SetStyle:
            updated = setElementStyleById(e.elementId, e.property, e.value);
            break;
        case HUDUpdateOperation::SetText:
            updated = setElementTextById(e.elementId, e.value);
            break;
        }

        if (!updated)
        {
            std::cerr << "[HUD] HUDUpdateEvent failed. elementId=" << e.elementId
                      << " operation=" << static_cast<uint32_t>(e.operation)
                      << " property=" << e.property
                      << " value=" << e.value << std::endl;
        }
    }

    void HudSystem::onActivateHUDEvent(const ActivateHUDEvent& e)
    {
        activateHud(e.hudKey);
    }

    void HudSystem::tick(int viewportWidth, int viewportHeight)
    {
        if (!mInitialized || !mContext || !mRenderInterface)
            return;

        int framebufferWidth = viewportWidth;
        int framebufferHeight = viewportHeight;
        glfwGetFramebufferSize(mWindow, &framebufferWidth, &framebufferHeight);

        float dpiScaleX = 1.0f;
        float dpiScaleY = 1.0f;
        glfwGetWindowContentScale(mWindow, &dpiScaleX, &dpiScaleY);
        mContext->SetDensityIndependentPixelRatio(dpiScaleX);

        if (!mLoggedFramebufferState)
        {
            GLint drawFramebuffer = 0;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
            std::cout << "[HUD] GL_DRAW_FRAMEBUFFER_BINDING before HUD render: " << drawFramebuffer << std::endl;
            std::cout << "[HUD] Framebuffer size=" << framebufferWidth << "x" << framebufferHeight
                      << " dpiScale=" << dpiScaleX << std::endl;
            mLoggedFramebufferState = true;
        }

        if (framebufferWidth > 0 && framebufferHeight > 0)
        {
            mRenderInterface->SetViewport(framebufferWidth, framebufferHeight);
            mContext->SetDimensions(Rml::Vector2i(framebufferWidth, framebufferHeight));
        }


        mContext->Update();

        // Start from a known-good state before entering the Rml renderer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, framebufferWidth, framebufferHeight);
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

        // Debug probe disabled now that HUD rendering is visible.
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // GLboolean scissorWasEnabled = GL_FALSE;
        // glGetBooleanv(GL_SCISSOR_TEST, &scissorWasEnabled);
        // glEnable(GL_SCISSOR_TEST);
        // const int probeSize = 64;
        // const int probeX = framebufferWidth > probeSize ? (framebufferWidth - probeSize) : 0;
        // const int probeY = framebufferHeight > probeSize ? (framebufferHeight - probeSize) : 0;
        // glScissor(probeX, probeY, probeSize, probeSize);
        // glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);
        // if (!scissorWasEnabled)
        //     glDisable(GL_SCISSOR_TEST);
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
        mEventBus = nullptr;
        mActiveHudKey.clear();

        mInitialized = false;
        gRmlInitialized = false;
        mLoggedFramebufferState = false;
    }
}




