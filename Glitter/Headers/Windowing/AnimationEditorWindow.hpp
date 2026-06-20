#pragma once

#include "Windowing/GameWindow.hpp"

#include <memory>
#include <string>
#include <vector>

#include "3DModel/Animation/Animation.hpp"
#include "Character/Character.hpp"
#include "Level/Level.hpp"
#include "Lights/Skybox.hpp"
#include "Lights/light.hpp"
#include "RenderPipeline/SceneViewport.hpp"

class AnimationEditorWindow final : public GameWindow
{
public:
    AnimationEditorWindow() = default;

    void init() override;
    void tickImpl() override;
    void shutdown() override;

private:
    struct ResourceEntry
    {
        std::string guid;
        std::string path;
    };

    std::vector<ResourceEntry> mCharacterEntries;
    std::vector<ResourceEntry> mAnimationEntries;
    int mSelectedCharacterIndex = -1;
    int mSelectedAnimationIndex = -1;

    std::string mSelectedCharacterGuid;
    std::string mSelectedAnimationGuid;

    std::shared_ptr<Character> mPreviewCharacter;
    Animation* mSelectedAnimation = nullptr;
    int mSelectedRegionIndex = -1;
    bool mLoopAnimation = true;
    bool mIsPlaying = false;
    float mScrubTimeSeconds = 0.0f;

    std::unique_ptr<Level> mPreviewLevel;
    std::unique_ptr<Lights> mLights;
    std::unique_ptr<Lighting::Skybox> mSkyBox;
    std::unique_ptr<SceneViewport> mSceneViewport;

    void setupPreviewScene();
    void refreshRegistryLists();
    void spawnPreviewCharacter(const std::string& characterGuid);
    void loadSelectedAnimation(const std::string& animationGuid);
    void updateAnimationPlaybackUI();
    void drawRegionEditorUI(float animationDurationSeconds);
    float getSelectedAnimationDurationSeconds() const;
};
