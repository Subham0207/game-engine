#pragma once

#include "Windowing/GameWindow.hpp"

#include <GLFW/glfw3.h>

#include <memory>

// Required because this header owns std::unique_ptr<NodeGraph>
#include <Debug/Raycast.hpp>

#include "RenderPipeline/SceneViewport.hpp"

#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"

#include "Level/Level.hpp"

#include "Event/EventBus.hpp"
#include "NodeGraph/StateMachineGraph.hpp"
#include "NodeGraph/FlowScript/FlowScript.hpp"
#include "NodeGraph/FlowScript/StatemachineFlowScript.hpp"

class EventQueue;
struct InputContext;
class InputHandler;

class Outliner;
namespace ProjectAsset { class AssetBrowser; }
// NodeGraph included above

// Separate State Machine tool window.
// Initially a copy of EditorWindow; later you can strip renderer/game tick.
class StateMachineWindow final : public GameWindow {
public:
    explicit StateMachineWindow(std::string characterFilePath, std::string statemachineFilePath);

    StateMachineWindow(const StateMachineWindow&) = delete;
    StateMachineWindow& operator=(const StateMachineWindow&) = delete;
    // This type owns unique_ptrs and also stores an EventBus; disable moves for simplicity.
    StateMachineWindow(StateMachineWindow&&) noexcept = delete;
    StateMachineWindow& operator=(StateMachineWindow&&) noexcept = delete;

    void init() override;
    void tickImpl() override;
    void shutdown() override;

    template<typename T>
    void load(T& t);

    void setSelectedModelIndex(const int idx){ selectedModelIndex = idx; }
    int getSelectedModelIndex() const { return selectedModelIndex; }

private:

    void setupLevelObjs();

    std::string mCharacterFilePath;
    std::string mStatemachineFilePath;

    int selectedModelIndex = 0;
    std::unique_ptr<Debug::Raycast> mRayCastObjectSelector;


    std::unique_ptr<SceneViewport> mSceneViewport;

    // Inline scene resources for this tool window (kept local for now).
    // This matches EditorWindow's pattern so we can render immediately without depending
    // on EditorWindow ownership.
    std::unique_ptr<Level> mPreviewLevel;
    std::unique_ptr<Lights> mLights;
    std::unique_ptr<Lighting::Skybox> mSkyBox;

    std::unique_ptr<StateMachineGraph> mStateMachineGraph;
    std::unique_ptr<StatemachineFlowScript> mSmFlowScript;
};

