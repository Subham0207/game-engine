#include <UI/AI_UI.hpp>
#include <imgui.h>
#include <EngineState.hpp>
#include <UI/Shared/ComboUI.hpp>
#include <UI/Shared/Utils.hpp>

#include "GenericFactory.hpp"
#include "Prefab.hpp"
#include "AI/AI.hpp"
#include "Modals/FileType.hpp"
#include "UI/Shared/EditableText.hpp"

namespace UI
{
    AI_UI::AI_UI()
    {
        showUI = false;
        aiPrefabName.setText("AI");
        aiPrefabRef = nullptr;
    }

    void AI_UI::start(const std::shared_ptr<AiPrefab>& aiPrefab, const std::string& aiMetaFilePath)
    {
        this->aiPrefabRef = aiPrefab;
        if (!aiMetaFilePath.empty())
        {
            auto guid = fs::path(aiMetaFilePath).filename().stem().stem().string();
            auto filename = fs::path(getEngineRegistryFilesMap()[guid]).filename().stem().string();
            this->aiPrefabName.setText(filename);
        }
        else
        {
            this->aiPrefabName.setText("AI");
        }

        int index = 0;
        aiClassNames = std::vector<std::string>();
        //Read all AI registered class names


        index = 0;
        characterPrefabNames = std::vector<std::string>();
        auto characterMap = EngineState::state->engineRegistry->characterPrefabMap;
        //Read all characterPrefabs
        for (auto& character: characterMap)
        {
            if (character.first == aiPrefab->characterPrefabAssetId)
            {
                aiPrefabUIState.selectedCharacterPrefabIndex = Utils::toUiIndex(index);
            }
            characterPrefabNames.emplace_back(character.second);
            index +=1;
        }
    }

    void AI_UI::draw()
    {
        if (!showUI)
            return;

        if (ImGui::Begin(this->aiPrefabName.value.c_str(), &showUI))
        {
            UI::Shared::EditableTextUI("Filename", aiPrefabName);

            UI::Shared::comboUI(
                "Choose AI class",
                aiPrefabUIState.selectedAiClassIndex,
                aiClassNames
                );

            UI::Shared::comboUI(
                "Choose AI class",
                aiPrefabUIState.selectedCharacterPrefabIndex,
                characterPrefabNames
                );

            this->save();

        }
        ImGui::End();
    }

    void AI_UI::save() const
    {
        if (ImGui::Button("Save"))
        {
            aiPrefabRef->name = this->aiPrefabName.value;
            aiPrefabRef->classId = aiClassNames[Utils::toDataTypeIndex(aiPrefabUIState.selectedAiClassIndex)];
            aiPrefabRef->characterPrefabAssetId = characterPrefabNames[Utils::toDataTypeIndex(aiPrefabUIState.selectedCharacterPrefabIndex)];

            auto filepath = EngineState::navIntoProjectDir("Assets"s + "/" + aiPrefabName.value + "." +  std::string(toString(FileType::AI)));
            Engine::Prefab::writeAIPrefab(filepath, aiPrefabRef);
        }
    }
}
