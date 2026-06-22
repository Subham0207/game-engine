#include "Windowing/AnimationEditorWindow.hpp"

#include <algorithm>
#include <filesystem>
#include <cstring>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include <EngineState.hpp>

#include "3DModel/model.hpp"
#include "Camera/Camera.hpp"
#include "Camera/FlyCam.hpp"
#include "Controls/Input.hpp"
#include "Helpers/Shared.hpp"

namespace
{
    std::string displayNameFromPath(const std::string& path)
    {
        const std::string name = std::filesystem::path(path).filename().stem().string();
        return name.empty() ? path : name;
    }
}

void AnimationEditorWindow::init()
{
    initWindowAndBackends(true, false, "Animation Editor", false);
    EngineState::state->isPlay = true;

    mPreviewLevel = std::make_unique<Level>();
    mPreviewLevel->setEventQueue(mQueue.get());
    mPreviewLevel->setEventBus(mBus.get());

    if (mCamera)
    {
        mInputHandler = std::make_unique<InputHandler>(mCamera.get(), mWindow, mScreenWidth, mScreenHeight);
        mPreviewLevel->setInputHandler(mInputHandler.get());
        mCamera->cameraPos = glm::vec3(0.0f, 2.0f, 8.0f);
        mCamera->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        mPreviewLevel->cameras.push_back(mCamera.get());
    }

    setupPreviewScene();

    const auto enginePath = std::filesystem::path(EngineState::state->engineInstalledDirectory);
    mSkyBox = std::make_unique<Lighting::Skybox>(enginePath, mWindow);

    mLights = std::make_unique<Lights>();
    mLights->initDefaultLights(mPreviewLevel.get());

    EngineState::state->GenerateDefaultMaterials();

    mSceneViewport = std::make_unique<SceneViewport>();
    mSceneViewport->init(mWindow, mLights.get());

    refreshRegistryLists();
}

void AnimationEditorWindow::setupPreviewScene()
{
    const auto enginePath = std::filesystem::path(EngineState::state->engineInstalledDirectory);
    auto ground = std::make_shared<Model>((enginePath / "EngineAssets" / "cube.fbx").string(), enginePath.string());
    ground->setTransform(glm::vec3(0.0f, -10.0f, 0.0f), glm::quat(), glm::vec3(100.0f, 1.0f, 100.0f));
    mPreviewLevel->renderables.push_back(ground);
}

void AnimationEditorWindow::refreshRegistryLists()
{
    mCharacterEntries.clear();
    mAnimationEntries.clear();

    if (EngineState::state == nullptr || EngineState::state->engineRegistry == nullptr)
        return;

    for (const auto& kv : EngineState::state->engineRegistry->characterPrefabMap)
        mCharacterEntries.push_back(ResourceEntry{kv.first, kv.second});

    for (const auto& kv : EngineState::state->engineRegistry->animationsFileMap)
        mAnimationEntries.push_back(ResourceEntry{kv.first, kv.second});

    auto syncSelection = [](const std::vector<ResourceEntry>& entries, const std::string& guid, int& selectedIndex)
    {
        selectedIndex = -1;
        for (int i = 0; i < static_cast<int>(entries.size()); ++i)
        {
            if (entries[i].guid == guid)
            {
                selectedIndex = i;
                return;
            }
        }
    };

    syncSelection(mCharacterEntries, mSelectedCharacterGuid, mSelectedCharacterIndex);
    syncSelection(mAnimationEntries, mSelectedAnimationGuid, mSelectedAnimationIndex);
}

void AnimationEditorWindow::spawnPreviewCharacter(const std::string& characterGuid)
{
    mSelectedCharacterGuid = characterGuid;

    if (mPreviewLevel == nullptr)
        return;

    if (mPreviewCharacter)
    {
        auto& renderables = mPreviewLevel->renderables;
        renderables.erase(
            std::remove_if(renderables.begin(), renderables.end(), [this](const std::shared_ptr<Renderable>& renderable)
            {
                return renderable.get() == mPreviewCharacter.get();
            }),
            renderables.end());
        mPreviewCharacter.reset();
    }

    const auto found = EngineState::state->engineRegistry->characterPrefabMap.find(characterGuid);
    if (found == EngineState::state->engineRegistry->characterPrefabMap.end())
        return;

    mPreviewCharacter = mPreviewLevel->spawnCharacter(found->second, glm::identity<glm::mat4>(), "", false);
    if (!mPreviewCharacter)
        return;

    mPreviewCharacter->setWorldTransform(glm::vec3(0.0f, 0.0f, 0.0f), glm::quat());
    if (mCamera)
    {
        mCamera->cameraPos = glm::vec3(0.0f, 2.0f, 6.0f);
        mCamera->cameraFront = glm::normalize(glm::vec3(0.0f, -0.1f, -1.0f));
    }

    if (!mSelectedAnimationGuid.empty())
        loadSelectedAnimation(mSelectedAnimationGuid);
}

void AnimationEditorWindow::loadSelectedAnimation(const std::string& animationGuid)
{
    mSelectedAnimationGuid = animationGuid;
    mSelectedAnimation = Animation::loadAnimation(animationGuid);
    mSelectedRegionIndex = -1;
    mScrubTimeSeconds = 0.0f;

    if (mPreviewCharacter == nullptr || mPreviewCharacter->animator == nullptr || mSelectedAnimation == nullptr)
        return;

    mPreviewCharacter->animator->PlayAnimation(mSelectedAnimation);
    mPreviewCharacter->animator->SetLoopCurrentAnimation(mLoopAnimation);
    mPreviewCharacter->animator->SetCurrentTimeSeconds(0.0f);
    mPreviewCharacter->animator->SetPlaying(mIsPlaying);
}

float AnimationEditorWindow::getSelectedAnimationDurationSeconds() const
{
    if (mSelectedAnimation == nullptr)
        return 0.0f;

    const float ticksPerSecond = glm::max(1.0f, mSelectedAnimation->GetTicksPerSecond());
    return mSelectedAnimation->GetDuration() / ticksPerSecond;
}

void AnimationEditorWindow::updateAnimationPlaybackUI()
{
    ImGui::SeparatorText("Playback");
    if (ImGui::Button("Play"))
    {
        mIsPlaying = true;
        if (mPreviewCharacter && mPreviewCharacter->animator)
            mPreviewCharacter->animator->SetPlaying(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause"))
    {
        mIsPlaying = false;
        if (mPreviewCharacter && mPreviewCharacter->animator)
            mPreviewCharacter->animator->SetPlaying(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        mIsPlaying = false;
        mScrubTimeSeconds = 0.0f;
        if (mPreviewCharacter && mPreviewCharacter->animator)
        {
            mPreviewCharacter->animator->SetPlaying(false);
            mPreviewCharacter->animator->SetCurrentTimeSeconds(0.0f);
        }
    }

    if (ImGui::Checkbox("Loop", &mLoopAnimation))
    {
        if (mPreviewCharacter && mPreviewCharacter->animator)
            mPreviewCharacter->animator->SetLoopCurrentAnimation(mLoopAnimation);
    }

    const float durationSeconds = getSelectedAnimationDurationSeconds();
    float scrubValue = mScrubTimeSeconds;
    if (durationSeconds > 0.0f)
    {
        if (ImGui::SliderFloat("Timeline (s)", &scrubValue, 0.0f, durationSeconds))
        {
            mScrubTimeSeconds = scrubValue;
            if (mPreviewCharacter && mPreviewCharacter->animator)
                mPreviewCharacter->animator->SetCurrentTimeSeconds(mScrubTimeSeconds);
        }
    }
    else
    {
        ImGui::TextDisabled("Timeline unavailable: select animation");
    }

    ImGui::Text("Time: %.3f / %.3f", mScrubTimeSeconds, durationSeconds);
    drawRegionEditorUI(durationSeconds);
}

void AnimationEditorWindow::drawRegionEditorUI(const float animationDurationSeconds)
{
    ImGui::SeparatorText("Regions");
    if (mSelectedAnimation == nullptr)
    {
        ImGui::TextDisabled("Select an animation to edit regions.");
        return;
    }

    if (ImGui::Button("Add Region"))
    {
        const float clamped = std::clamp(mScrubTimeSeconds, 0.0f, animationDurationSeconds);
        mSelectedAnimation->regions.push_back(AnimationRegion{"Region", clamped, clamped});
        mSelectedRegionIndex = static_cast<int>(mSelectedAnimation->regions.size()) - 1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Save Regions"))
        mSelectedAnimation->saveRegionsToMeta();

    if (mSelectedRegionIndex >= 0 && mSelectedRegionIndex < static_cast<int>(mSelectedAnimation->regions.size()))
    {
        if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            mSelectedAnimation->regions.erase(mSelectedAnimation->regions.begin() + mSelectedRegionIndex);
            if (mSelectedRegionIndex >= static_cast<int>(mSelectedAnimation->regions.size()))
                mSelectedRegionIndex = static_cast<int>(mSelectedAnimation->regions.size()) - 1;
        }
    }

    ImGui::BeginChild("RegionList", ImVec2(0.0f, 220.0f), true);
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    for (int i = 0; i < static_cast<int>(mSelectedAnimation->regions.size()); ++i)
    {
        auto& region = mSelectedAnimation->regions[i];
        ImGui::PushID(i);
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        const bool selected = (mSelectedRegionIndex == i);
        if (ImGui::Selectable("##region_select", selected))
            mSelectedRegionIndex = i;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(180.0f);
        char nameBuffer[128] = {};
        strncpy_s(nameBuffer, sizeof(nameBuffer), region.name.c_str(), _TRUNCATE);
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
            region.name = nameBuffer;
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("Start", &region.startTime, 0.01f, 0.0f, animationDurationSeconds))
            region.startTime = std::clamp(region.startTime, 0.0f, animationDurationSeconds);
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("End", &region.endTime, 0.01f, 0.0f, animationDurationSeconds))
            region.endTime = std::clamp(region.endTime, 0.0f, animationDurationSeconds);
        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::EndChild();

    const ImVec2 timelineSize(ImGui::GetContentRegionAvail().x, 36.0f);
    const ImVec2 timelineMin = ImGui::GetCursorScreenPos();
    const ImVec2 timelineMax(timelineMin.x + timelineSize.x, timelineMin.y + timelineSize.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(timelineMin, timelineMax, IM_COL32(40, 40, 40, 255));

    if (animationDurationSeconds > 0.0f)
    {
        for (int i = 0; i < static_cast<int>(mSelectedAnimation->regions.size()); ++i)
        {
            const auto& region = mSelectedAnimation->regions[i];
            const float normalizedStart = std::clamp(region.startTime / animationDurationSeconds, 0.0f, 1.0f);
            const float normalizedEnd = std::clamp(region.endTime / animationDurationSeconds, 0.0f, 1.0f);
            const float x0 = timelineMin.x + normalizedStart * timelineSize.x;
            const float x1 = timelineMin.x + normalizedEnd * timelineSize.x;
            const ImU32 color = (mSelectedRegionIndex == i) ? IM_COL32(240, 180, 40, 220) : IM_COL32(80, 170, 240, 180);
            drawList->AddRectFilled(ImVec2(std::min(x0, x1), timelineMin.y + 4.0f), ImVec2(std::max(x0, x1), timelineMax.y - 4.0f), color);
        }
    }

    ImGui::InvisibleButton("regions_timeline", timelineSize);
}

void AnimationEditorWindow::tickImpl()
{
    if (!mWindow)
        return;

    makeCurrent();
    setImguiCurrent();
    if (mImNodesContext)
        ImNodes::SetCurrentContext(mImNodesContext);

    if (mInputHandler)
        mInputHandler->handleInput(mDeltaTime, *mInputCtx, false);

    if (mPreviewCharacter && mPreviewCharacter->animator && mSelectedAnimation)
    {
        mPreviewCharacter->animator->SetPlaying(mIsPlaying);
        mPreviewCharacter->animator->SetLoopCurrentAnimation(mLoopAnimation);
        mPreviewCharacter->updateFinalBoneMatrix(mDeltaTime);
        mScrubTimeSeconds = mPreviewCharacter->animator->GetCurrentTimeSeconds();
    }

    Camera* activeCamera = mCamera.get();
    if (mSceneViewport && mPreviewLevel && activeCamera && mScreenWidth > 0 && mScreenHeight > 0)
    {
        mSceneViewport->resize(mScreenWidth, mScreenHeight);
        activeCamera->setFrameContext(frameContext(mScreenWidth, mScreenHeight));
        activeCamera->tick();
        mSceneViewport->render(
            mPreviewLevel->renderables,
            activeCamera,
            mLights.get(),
            mSkyBox.get(),
            mDeltaTime);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    refreshRegistryLists();
    ImGui::Begin("Animation Editor");
    {
        std::vector<std::string> characterLabels;
        characterLabels.reserve(mCharacterEntries.size());
        for (const auto& entry : mCharacterEntries)
            characterLabels.push_back(displayNameFromPath(entry.path));

        std::vector<const char*> characterItems;
        characterItems.reserve(characterLabels.size());
        for (const auto& name : characterLabels)
            characterItems.push_back(name.c_str());

        if (ImGui::Combo("Character", &mSelectedCharacterIndex, characterItems.data(), static_cast<int>(characterItems.size())))
        {
            if (mSelectedCharacterIndex >= 0 && mSelectedCharacterIndex < static_cast<int>(mCharacterEntries.size()))
                spawnPreviewCharacter(mCharacterEntries[mSelectedCharacterIndex].guid);
        }

        std::vector<std::string> animationLabels;
        animationLabels.reserve(mAnimationEntries.size());
        for (const auto& entry : mAnimationEntries)
            animationLabels.push_back(displayNameFromPath(entry.path));

        std::vector<const char*> animationItems;
        animationItems.reserve(animationLabels.size());
        for (const auto& name : animationLabels)
            animationItems.push_back(name.c_str());

        if (ImGui::Combo("Animation", &mSelectedAnimationIndex, animationItems.data(), static_cast<int>(animationItems.size())))
        {
            if (mSelectedAnimationIndex >= 0 && mSelectedAnimationIndex < static_cast<int>(mAnimationEntries.size()))
                loadSelectedAnimation(mAnimationEntries[mSelectedAnimationIndex].guid);
        }

        updateAnimationPlaybackUI();
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(mWindow);
}

void AnimationEditorWindow::shutdown()
{
    if (!mWindow)
        return;

    makeCurrent();
    if (mImguiContext)
        ImGui::SetCurrentContext(mImguiContext);
    if (mImNodesContext)
        ImNodes::SetCurrentContext(mImNodesContext);

    Shared::shutdownImguiBackendForWindow();
    if (mImNodesContext)
    {
        ImNodes::DestroyContext(mImNodesContext);
        mImNodesContext = nullptr;
    }
    if (mImguiContext)
    {
        ImGui::DestroyContext(mImguiContext);
        mImguiContext = nullptr;
    }

    glfwDestroyWindow(mWindow);
    mWindow = nullptr;
}
