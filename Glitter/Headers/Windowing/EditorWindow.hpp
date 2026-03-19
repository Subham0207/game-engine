#pragma once

#include "Windowing/GameWindow.hpp"

#include <memory>

// MSVC requires complete types for std::unique_ptr<T> members when the owning
// class destructor is instantiated in translation units that only include this
// header (e.g. via std::make_unique<EditorWindow> in Editor.cpp).
// Including these here is the simplest and most reliable fix.
#include "RenderPipeline/ShadowPass.hpp"
#include "RenderPipeline/LightingPass.hpp"
#include "RenderPipeline/PostProcess.hpp"

#include "Level/Level.hpp"
#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"
#include "Debug/Raycast.hpp"
#include "UI/outliner.hpp"
#include "UI/AssetBrowser/AssetBrowser.hpp"
#include "NodeGraph/NodeGraph.hpp"

#include "Controls/ClientHandler.hpp"

// NOTE:
// This header intentionally uses forward declarations for most engine types.
// `EditorWindow`'s destructor is defined out-of-line in EditorWindow.cpp so
// std::unique_ptr members can be destroyed without requiring full type
// definitions here.

class EventQueue;
class InputHandler;
struct InputContext;

class Level;
namespace Lighting { class Skybox; }
class Lights;
namespace Debug { class Raycast; }
class Outliner;
namespace ProjectAsset { class AssetBrowser; }
class NodeGraph;

class ShadowPass;
class LightingPass;
class PostProcess;

// The main editor window (former content of Editor::openEditor loop).
class EditorWindow final : public GameWindow {
public:
    ~EditorWindow() override;

    EditorWindow() = default;
    EditorWindow(const EditorWindow&) = delete;
    EditorWindow& operator=(const EditorWindow&) = delete;
    EditorWindow(EditorWindow&&) noexcept = default;
    EditorWindow& operator=(EditorWindow&&) noexcept = default;

    void init() override;
    void tickImpl() override;
    void shutdown() override;

private:

    std::unique_ptr<Level> mLevel;
    std::unique_ptr<Lighting::Skybox> mSkyBox;
    std::unique_ptr<Debug::Raycast> mRayCastObjectSelector;
    std::unique_ptr<Lights> mLights;

    std::unique_ptr<Outliner> mOutliner;
    std::unique_ptr<ProjectAsset::AssetBrowser> mAssetBrowser;
    std::unique_ptr<NodeGraph> mNodeGraph;

    std::unique_ptr<ShadowPass> mShadowPass;
    std::unique_ptr<LightingPass> mLightingPass;
    std::unique_ptr<PostProcess> mPostProcess;

    std::unique_ptr<ClientHandler> mClientHandler;

    bool mFirstFrame = false;
};

