#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

#include "GenericFactory.hpp"
#include "Prefab.hpp"
#include "UI/Shared/ComboUI.hpp"

UI::CharacterUI::CharacterUI() : characterConfig()
{
    showCharacterUI = false;
    characterName = "Character UI";
    characterPrefabConfig = nullptr;
}

void UI::CharacterUI::start(CharacterPrefabConfig& characterPrefab, std::string characterName)
{
    this->characterPrefabConfig = &characterPrefab;
    this->characterName = characterName;

    registeredClassNames = std::vector<std::string>();
    int index = 0;
    for (const auto& [fst, snd] : CharacterFactory::GetTable())
    {
        if(fst == characterPrefab.classId)
        {
            characterConfig.selectedRegisteredCharacterIndex = index;
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
        if (filename == characterPrefab.modelGuid)
        {
            characterConfig.selectedModelIndex = index;
        }
        index+=1;
        modelNames.emplace_back(filename);
    }

    auto skeletonMap = EngineState::state->engineRegistry->skeletonFileMap;
    skeletonNames = std::vector<std::string>();
    index = 0;
    for (auto& skeleton : skeletonMap)
    {
        auto filename = skeleton.second;
        if (filename == characterPrefab.skeletonGuid)
        {
            characterConfig.selectedSkeletonIndex = index;
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
            characterConfig.selectedStateMachineIndex = index;
        }
        index+=1;
        statemachineNames.emplace_back(fst);
    }

    showCharacterUI = true;
}
void UI::CharacterUI::draw()
{
    if (ImGui::Begin(characterName.c_str(), &showCharacterUI))
    {
        //ClassId:  List all keys of CharactorFactory to choose from.
        //model_guid: List all the .model files from Engine registry.
        //skeleton_guid: list all .skeleton files from Engine registry.
        //statemachineClassId: List All keys from statemachineFactory.
        ;
        UI::Shared::comboUI(
            "Choose Character Class",
            characterConfig.selectedRegisteredCharacterIndex,
            registeredClassNames
            );

        UI::Shared::comboUI(
            "Choose a model",
            characterConfig.selectedModelIndex,
            modelNames
            );

        UI::Shared::comboUI(
            "Choose a skeletal",
            characterConfig.selectedSkeletonIndex,
            skeletonNames
            );

        UI::Shared::comboUI(
            "Choose a statemachine",
            characterConfig.selectedStateMachineIndex,
            statemachineNames
        );

        //After all selection is made. save it to character.prefab.
        if (ImGui::Button("Save"))
        {
            characterPrefabConfig->classId = registeredClassNames[characterConfig.selectedRegisteredCharacterIndex];
            auto model_guid = EngineState::state->engineRegistry->modelFileMap[modelNames[characterConfig.selectedModelIndex]];
            auto skeleton_guid = EngineState::state->engineRegistry->skeletonFileMap[skeletonNames[characterConfig.selectedSkeletonIndex]];
            characterPrefabConfig->modelGuid = model_guid;
            characterPrefabConfig->skeletonGuid = skeleton_guid;
            characterPrefabConfig->stateMachineClassId = statemachineNames[characterConfig.selectedStateMachineIndex];

            auto filepath = EngineState::navIntoProjectDir("Assets"s + characterName);
            Engine::Prefab::writeCharacterPrefab(filepath, *characterPrefabConfig);
        }

    }
    ImGui::End();
}