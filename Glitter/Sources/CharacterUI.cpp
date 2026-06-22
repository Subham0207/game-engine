#include <UI/CharacterUI.hpp>
#include <imgui.h>
#include <EngineState.hpp>

#include "GenericFactory.hpp"
#include "Prefab.hpp"
#include "Modals/FileType.hpp"
#include "Physics/PhysicsLayerRegistry.hpp"
#include "UI/Shared/ComboUI.hpp"
#include "UI/Shared/Utils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
    std::vector<std::string> parseTagsCsv(const std::string& csv)
    {
        std::vector<std::string> tags;
        std::stringstream stream(csv);
        std::string token;
        while (std::getline(stream, token, ','))
        {
            token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), token.end());
            if (!token.empty())
            {
                tags.push_back(token);
            }
        }
        return tags;
    }

    std::string toTagsCsv(const std::vector<std::string>& tags)
    {
        std::string csv;
        for (size_t i = 0; i < tags.size(); ++i)
        {
            csv += tags[i];
            if (i + 1 < tags.size())
            {
                csv += ", ";
            }
        }
        return csv;
    }
}

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
    characterConfigUIModel.capsuleIsSensor = characterPrefab.capsuleIsSensor;
    characterConfigUIModel.capsuleMass = characterPrefab.capsuleMass;
    characterConfigUIModel.capsuleMaxStrength = characterPrefab.capsuleMaxStrength;
    characterConfigUIModel.capsuleFriction = characterPrefab.capsuleFriction;
    characterConfigUIModel.capsuleRestitution = characterPrefab.capsuleRestitution;
    characterConfigUIModel.modelScale = characterPrefab.modelScale;

    auto& layerRegistry = Physics::PhysicsLayerRegistry::instance();
    const auto& layerNames = layerRegistry.getLayerNames();
    auto foundLayer = std::find(layerNames.begin(), layerNames.end(), characterPrefab.capsulePhysicsLayer);
    if (foundLayer == layerNames.end())
    {
        characterConfigUIModel.selectedCapsulePhysicsLayerIndex = 0;
    }
    else
    {
        characterConfigUIModel.selectedCapsulePhysicsLayerIndex = static_cast<int>(std::distance(layerNames.begin(), foundLayer));
    }

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

    auto statemachineMap = EngineState::state->engineRegistry->statemachineFileMap;
    statemachineNames = std::vector<std::string>();
    index=0;
    for (auto& statemachine : statemachineMap)
    {
        auto filename = statemachine.second;
        if (statemachine.first == characterPrefab.stateMachineGuid)
        {
            characterConfigUIModel.selectedStateMachineIndex = Utils::toUiIndex(index);
        }
        index+=1;
        statemachineNames.emplace_back(filename);
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
        {
            auto& layerRegistry = Physics::PhysicsLayerRegistry::instance();
            const auto& layerNames = layerRegistry.getLayerNames();
            std::vector<const char*> layerNamePtrs;
            layerNamePtrs.reserve(layerNames.size());
            for (const auto& name : layerNames)
            {
                layerNamePtrs.push_back(name.c_str());
            }

            if (!layerNamePtrs.empty())
            {
                if (characterConfigUIModel.selectedCapsulePhysicsLayerIndex < 0 ||
                    characterConfigUIModel.selectedCapsulePhysicsLayerIndex >= static_cast<int>(layerNamePtrs.size()))
                {
                    characterConfigUIModel.selectedCapsulePhysicsLayerIndex = 0;
                }

                ImGui::Combo(
                    "Capsule Physics Layer",
                    &characterConfigUIModel.selectedCapsulePhysicsLayerIndex,
                    layerNamePtrs.data(),
                    static_cast<int>(layerNamePtrs.size()));
            }
        }
        ImGui::Checkbox("Capsule Is Sensor", &characterConfigUIModel.capsuleIsSensor);
        ImGui::DragFloat("Capsule Mass", &characterConfigUIModel.capsuleMass, 1.0f, 0.001f, 10000.0f);
        ImGui::DragFloat("Capsule Max Strength", &characterConfigUIModel.capsuleMaxStrength, 1.0f, 0.0f, 100000.0f);
        ImGui::DragFloat("Capsule Friction", &characterConfigUIModel.capsuleFriction, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Capsule Restitution", &characterConfigUIModel.capsuleRestitution, 0.01f, 0.0f, 1.0f);

        char tagsBuffer[512]{};
        const std::string tagsCsv = toTagsCsv(characterPrefabConfig->gameplayTags);
        const size_t copyCount = std::min(tagsCsv.size(), sizeof(tagsBuffer) - 1);
        std::copy(tagsCsv.begin(), tagsCsv.begin() + static_cast<std::string::difference_type>(copyCount), tagsBuffer);
        if (ImGui::InputText("Gameplay Tags (csv)", tagsBuffer, sizeof(tagsBuffer)))
        {
            characterPrefabConfig->gameplayTags = parseTagsCsv(tagsBuffer);
        }
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

            auto& statemachineFileMap = EngineState::state->engineRegistry->statemachineFileMap;
            auto statemachine_guid = std::find_if(statemachineFileMap.begin(), statemachineFileMap.end(), [&](const auto& pair)
            {
                return pair.second == statemachineNames[characterConfigUIModel.selectedStateMachineIndex - 1];
            })->first;

            characterPrefabConfig->modelGuid = model_guid;
            characterPrefabConfig->modelRelativePosition = characterConfigUIModel.modelRelativePosition;
            characterPrefabConfig->modelScale = characterConfigUIModel.modelScale;

            characterPrefabConfig->capsuleHalfHeight = characterConfigUIModel.capsuleHalfHeight;
            characterPrefabConfig->capsuleRadius = characterConfigUIModel.capsuleRadius;
            auto& layerRegistry = Physics::PhysicsLayerRegistry::instance();
            const auto& layerNames = layerRegistry.getLayerNames();
            if (!layerNames.empty())
            {
                const int clampedLayerIndex = std::max(0, std::min(characterConfigUIModel.selectedCapsulePhysicsLayerIndex, static_cast<int>(layerNames.size()) - 1));
                characterPrefabConfig->capsulePhysicsLayer = layerNames[static_cast<size_t>(clampedLayerIndex)];
            }
            else
            {
                characterPrefabConfig->capsulePhysicsLayer = "Default";
            }
            characterPrefabConfig->capsuleIsSensor = characterConfigUIModel.capsuleIsSensor;
            characterPrefabConfig->capsuleMass = std::max(0.001f, characterConfigUIModel.capsuleMass);
            characterPrefabConfig->capsuleMaxStrength = std::max(0.0f, characterConfigUIModel.capsuleMaxStrength);
            characterPrefabConfig->capsuleFriction = std::max(0.0f, characterConfigUIModel.capsuleFriction);
            characterPrefabConfig->capsuleRestitution = std::max(0.0f, std::min(1.0f, characterConfigUIModel.capsuleRestitution));

            characterPrefabConfig->skeletonGuid = skeleton_guid;
            characterPrefabConfig->stateMachineGuid = statemachine_guid;
            characterPrefabConfig->controllerClassId = controllerNames[characterConfigUIModel.selectedControllerIndex - 1];

            auto filepath = EngineState::navIntoProjectDir("Assets"s + "/" + characterName.value + "." +  std::string(toString(FileType::CharacterType)));
            Engine::Prefab::writeCharacterPrefab(filepath, *characterPrefabConfig);
        }

    }
    ImGui::End();
}
