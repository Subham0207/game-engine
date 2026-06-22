#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "Event/Event.hpp"

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

class SystemInterface_GLFW;
class RenderInterface_GL3;
class EventBus;

namespace Rml
{
    class Context;
    class ElementDocument;
}

namespace UI::Hud
{
    class HudSystem
    {
    public:
        struct HudDefinition
        {
            std::filesystem::path rmlPath;
            std::filesystem::path rcssPath;
        };

        HudSystem() = default;
        ~HudSystem();

        HudSystem(const HudSystem&) = delete;
        HudSystem& operator=(const HudSystem&) = delete;
        HudSystem(HudSystem&&) noexcept = delete;
        HudSystem& operator=(HudSystem&&) noexcept = delete;

        bool init(GLFWwindow* window, int initialWidth, int initialHeight, EventBus* eventBus);
        void tick(int viewportWidth, int viewportHeight);
        void shutdown();

        void discoverHudDocuments(const std::filesystem::path& hudDirectory);
        void registerHudDocument(const std::string& key, const std::filesystem::path& rmlPath, const std::filesystem::path& rcssPath = {});
        bool activateHud(const std::string& key);
        bool setElementStyleById(const std::string& id, const std::string& property, const std::string& value);
        bool setElementTextById(const std::string& id, const std::string& value);

        [[nodiscard]] bool isInitialized() const { return mInitialized; }

    private:
        void onHUDUpdateEvent(const HUDUpdateEvent& e);
        void onActivateHUDEvent(const ActivateHUDEvent& e);

        GLFWwindow* mWindow = nullptr;
        EventBus* mEventBus = nullptr;
        SystemInterface_GLFW* mSystemInterface = nullptr;
        RenderInterface_GL3* mRenderInterface = nullptr;
        Rml::Context* mContext = nullptr;
        Rml::ElementDocument* mDocument = nullptr;
        std::string mActiveHudKey;
        std::unordered_map<std::string, HudDefinition> mHudRegistry;
        bool mInitialized = false;
        bool mLoggedFramebufferState = false;
    };
}


