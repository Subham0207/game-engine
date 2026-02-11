#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

#include "GenericFactory.hpp"
#include "Prefab.hpp"
#include "Modals/FileType.hpp"
#include "UI/Shared/ComboUI.hpp"

UI::CharacterUI::CharacterUI() : characterConfigUIModel()
{
    showCharacterUI = false;
    characterName.setText("Character");
    characterPrefabConfig = nullptr;
}

void UI::CharacterUI::start(CharacterPrefabConfig& characterPrefab, std::string characterMetaFilePath)
{
    this->characterPrefabConfig = &characterPrefab;
    if (!characterMetaFilePath.empty())
    {
        auto guid = fs::path(characterMetaFilePath).filename().stem().stem().string();
        auto filename = fs::path(getEngineRegistryFilesMap()[guid]).filename().stem().string();
        this->characterName.setText(filename);
    }

    registeredClassNames = std::vector<std::string>();
    int index = 0;
    for (const auto& [fst, snd] : CharacterFactory::GetTable())
    {
        if(fst == characterPrefab.classId)
        {
            characterConfigUIModel.selectedRegisteredCharacterIndex = toUiIndex(index);
        }
        index+=1;
        registeredClassNames.emplace_back(fst);
    }

    auto modelMap = EngineState::state->engineRegistry->modelFileMap;
    modelNames = std::vector<std::string>();
    index = 0;
    for (auto& model : modelMap)
    {
        auto filename = model.second;
        if (model.first == characterPrefab.modelGuid)
        {
            characterConfigUIModel.selectedModelIndex = toUiIndex(index);
        }
        index+=1;
        modelNames.emplace_back(filename);
    }
    characterConfigUIModel.modelRelativePosition = characterPrefab.modelRelativePosition;
    characterConfigUIModel.capsuleHalfHeight = characterPrefab.capsuleHalfHeight;
    characterConfigUIModel.capsuleRadius = characterPrefab.capsuleRadius;

    auto skeletonMap = EngineState::state->engineRegistry->skeletonFileMap;
    skeletonNames = std::vector<std::string>();
    index = 0;
    for (auto& skeleton : skeletonMap)
    {
        auto filename = skeleton.second;
        if (skeleton.first == characterPrefab.skeletonGuid)
        {
            characterConfigUIModel.selectedSkeletonIndex = toUiIndex(index);
        }
        index+=1;
        skeletonNames.emplace_back(filename);
    }

    statemachineNames = std::vector<std::string>();
    index=0;
    for (const auto& [fst, snd] : StateMachineFactory::GetTable())
    {
        if (fst == characterPrefab.stateMachineClassId)
        {
            characterConfigUIModel.selectedStateMachineIndex = toUiIndex(index);
        }
        index+=1;
        statemachineNames.emplace_back(fst);
    }

    playerControllerNames = std::vector<std::string>();
    index=0;
    for (const auto& [fst, snd] : PlayerControllerFactory::GetTable())
    {
        if (fst == characterPrefab.playerControllerClassId)
        {
            characterConfigUIModel.selectedPlayerControllerIndex = toUiIndex(index);
        }
        index+=1;
        playerControllerNames.emplace_back(fst);
    }

    showCharacterUI = true;
}

int UI::CharacterUI::toUiIndex(int dataTypeIndex)
{
    // 0th is reserved for None, From 1st index your data starts.
    return dataTypeIndex + 1;
}

int UI::CharacterUI::toDataTypeIndex(int UiIndex)
{
    // None will be 0th index so don't call this function.
    return UiIndex - 1;
}

void UI::CharacterUI::draw()
{
    if (!showCharacterUI)
        return;

    if (ImGui::Begin(this->characterName.value.c_str(), &showCharacterUI))
    {
        //ClassId:  List all keys of CharactorFactory to choose from.
        //model_guid: List all the .model files from Engine registry.
        //skeleton_guid: list all .skeleton files from Engine registry.
        //statemachineClassId: List All keys from statemachineFactory.
        ;

        UI::Shared::EditableTextUI("filename", characterName);

        UI::Shared::comboUI(
            "Choose Character Class",
            characterConfigUIModel.selectedRegisteredCharacterIndex,
            registeredClassNames
            );

        UI::Shared::comboUI(
            "Choose a model",
            characterConfigUIModel.selectedModelIndex,
            modelNames
            );

        ImGui::DragFloat3("Vector Position", &characterConfigUIModel.modelRelativePosition.x, 0.1f);
        ImGui::SliderFloat("Capsule Half Height", &characterConfigUIModel.capsuleHalfHeight, 0.0f, 100.0f);
        ImGui::SliderFloat("Capsule Radius", &characterConfigUIModel.capsuleRadius, 0.0f, 100.0f);

        UI::Shared::comboUI(
            "Choose a skeletal",
            characterConfigUIModel.selectedSkeletonIndex,
            skeletonNames
            );

        UI::Shared::comboUI(
            "Choose a statemachine",
            characterConfigUIModel.selectedStateMachineIndex,
            statemachineNames
        );

        UI::Shared::comboUI(
            "Choose a PlayerController",
            characterConfigUIModel.selectedPlayerControllerIndex,
            playerControllerNames
        );

        //After all selection is made. save it to character.prefab.
        if (ImGui::Button("Save"))
        {
            characterPrefabConfig->classId = registeredClassNames[characterConfigUIModel.selectedRegisteredCharacterIndex - 1];

            auto modelFileMap = EngineState::state->engineRegistry->modelFileMap;
            auto model_guid = std::find_if(modelFileMap.begin(), modelFileMap.end(), [&](const auto& pair)
            {
                return pair.second == modelNames[characterConfigUIModel.selectedModelIndex - 1];
            })->first;

            auto skeletonFileMap = EngineState::state->engineRegistry->skeletonFileMap;
            auto skeleton_guid = std::find_if(skeletonFileMap.begin(), skeletonFileMap.end(), [&](const auto& pair)
            {
                return pair.second == skeletonNames[characterConfigUIModel.selectedSkeletonIndex - 1];
            })->first;

            characterPrefabConfig->modelGuid = model_guid;
            characterPrefabConfig->modelRelativePosition = characterConfigUIModel.modelRelativePosition;
            characterPrefabConfig->capsuleHalfHeight = characterConfigUIModel.capsuleHalfHeight;
            characterPrefabConfig->capsuleRadius = characterConfigUIModel.capsuleRadius;

            characterPrefabConfig->skeletonGuid = skeleton_guid;
            characterPrefabConfig->stateMachineClassId = statemachineNames[characterConfigUIModel.selectedStateMachineIndex - 1];
            characterPrefabConfig->playerControllerClassId = playerControllerNames[characterConfigUIModel.selectedPlayerControllerIndex - 1];

            auto filepath = EngineState::navIntoProjectDir("Assets"s + "/" + characterName.value + "." +  std::string(toString(FileType::CharacterType)));
            Engine::Prefab::writeCharacterPrefab(filepath, *characterPrefabConfig);
        }

    }
    ImGui::End();
}
