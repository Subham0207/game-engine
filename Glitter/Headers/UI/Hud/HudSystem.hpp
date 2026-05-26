#pragma once

#include <filesystem>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

class SystemInterface_GLFW;
class RenderInterface_GL3;

namespace Rml
{
    class Context;
    class ElementDocument;
    class FontEngineInterface;
}

namespace UI::Hud
{
    class HudSystem
    {
    public:
        HudSystem() = default;
        ~HudSystem();

        HudSystem(const HudSystem&) = delete;
        HudSystem& operator=(const HudSystem&) = delete;
        HudSystem(HudSystem&&) noexcept = delete;
        HudSystem& operator=(HudSystem&&) noexcept = delete;

        bool init(GLFWwindow* window, int initialWidth, int initialHeight, const std::filesystem::path& documentPath);
        void tick(int viewportWidth, int viewportHeight);
        void shutdown();

        [[nodiscard]] bool isInitialized() const { return mInitialized; }

    private:
        GLFWwindow* mWindow = nullptr;
        SystemInterface_GLFW* mSystemInterface = nullptr;
        RenderInterface_GL3* mRenderInterface = nullptr;
        Rml::Context* mContext = nullptr;
        Rml::ElementDocument* mDocument = nullptr;
        Rml::FontEngineInterface* mFallbackFontEngine = nullptr;
        bool mInitialized = false;
        bool mLoggedFramebufferState = false;
    };
}


