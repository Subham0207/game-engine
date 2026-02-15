#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

#include "GenericFactory.hpp"
#include "Prefab.hpp"
#include "Modals/FileType.hpp"
#include "UI/Shared/ComboUI.hpp"
#include "UI/Shared/Utils.hpp"

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
    else
    {
        this->characterName.setText("Character");
    }

    registeredClassNames = std::vector<std::string>();
    int index = 0;
    for (const auto& [fst, snd] : CharacterFactory::GetTable())
    {
        if(fst == characterPrefab.classId)
        {
            characterConfigUIModel.selectedRegisteredCharacterIndex = Utils::toUiIndex(index);
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
            characterConfigUIModel.selectedModelIndex = Utils::toUiIndex(index);
        }
        index+=1;
        modelNames.emplace_back(filename);
    }
    characterConfigUIModel.modelRelativePosition = characterPrefab.modelRelativePosition;
    characterConfigUIModel.capsuleHalfHeight = characterPrefab.capsuleHalfHeight;
    characterConfigUIModel.capsuleRadius = characterPrefab.capsuleRadius;
    characterConfigUIModel.modelScale = characterPrefab.modelScale;

    auto skeletonMap = EngineState::state->engineRegistry->skeletonFileMap;
    skeletonNames = std::vector<std::string>();
    index = 0;
    for (auto& skeleton : skeletonMap)
    {
        auto filename = skeleton.second;
        if (skeleton.first == characterPrefab.skeletonGuid)
        {
            characterConfigUIModel.selectedSkeletonIndex = Utils::toUiIndex(index);
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
            characterConfigUIModel.selectedStateMachineIndex = Utils::toUiIndex(index);
        }
        index+=1;
        statemachineNames.emplace_back(fst);
    }

    controllerNames = std::vector<std::string>();
    index=0;
    for (const auto& [fst, snd] : ControllerFactory::GetTable())
    {
        if (fst == characterPrefab.controllerClassId)
        {
            characterConfigUIModel.selectedControllerIndex = Utils::toUiIndex(index);
        }
        index+=1;
        controllerNames.emplace_back(fst);
    }

    showCharacterUI = true;
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

        ImGui::DragFloat3("Model Relative Position", &characterConfigUIModel.modelRelativePosition.x, 0.1f);
        ImGui::DragFloat("Capsule Half Height", &characterConfigUIModel.capsuleHalfHeight, 0.1f);
        ImGui::DragFloat("Capsule Radius", &characterConfigUIModel.capsuleRadius, 0.1f);
        ImGui::DragFloat3("Model Scale", &characterConfigUIModel.modelScale.x, 0.1f);

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
            characterConfigUIModel.selectedControllerIndex,
            controllerNames
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
            characterPrefabConfig->modelScale = characterConfigUIModel.modelScale;

            characterPrefabConfig->capsuleHalfHeight = characterConfigUIModel.capsuleHalfHeight;
            characterPrefabConfig->capsuleRadius = characterConfigUIModel.capsuleRadius;

            characterPrefabConfig->skeletonGuid = skeleton_guid;
            characterPrefabConfig->stateMachineClassId = statemachineNames[characterConfigUIModel.selectedStateMachineIndex - 1];
            characterPrefabConfig->controllerClassId = controllerNames[characterConfigUIModel.selectedControllerIndex - 1];

            auto filepath = EngineState::navIntoProjectDir("Assets"s + "/" + characterName.value + "." +  std::string(toString(FileType::CharacterType)));
            Engine::Prefab::writeCharacterPrefab(filepath, *characterPrefabConfig);
        }

    }
    ImGui::End();
}
