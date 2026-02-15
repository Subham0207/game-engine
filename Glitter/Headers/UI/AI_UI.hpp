#pragma once
#include <memory>
#include <string>
#include <vector>

#include "AI/AiPrefab.hpp"
#include "Shared/EditableText.hpp"

class Character;

namespace UI
{
    struct AiPrefabUIState
    {
        int selectedAiClassIndex;

        int selectedCharacterPrefabIndex;
    };

    class AI_UI
    {
        public:
            AI_UI();
            void start(const std::shared_ptr<AiPrefab>& aiPrefab, const std::string& aiMetaFilePath="");
            void draw();
            bool showUI;
        private:
            void save() const;

            std::shared_ptr<AiPrefab> aiPrefabRef;

            AiPrefabUIState aiPrefabUIState;
            Shared::EditableText aiPrefabName;

            std::vector<std::string> aiClassNames;
            std::vector<std::string> characterPrefabNames;
    };
}