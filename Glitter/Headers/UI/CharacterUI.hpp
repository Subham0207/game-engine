#include <Character/Character.hpp>

#include "Character/CharacterPrefabConfig.hpp"
#include "Shared/EditableText.hpp"

namespace UI
{
    struct CharacterConfigUIModel
    {
        int selectedRegisteredCharacterIndex;
        int selectedModelIndex;
        int selectedSkeletonIndex;
        int selectedStateMachineIndex;
    };

    class CharacterUI
    {
    public:
        CharacterUI();
        void draw();
        void start(CharacterPrefabConfig& characterPrefab, std::string characterName = "Character UI");
        bool showCharacterUI;
    private:
        Shared::EditableText characterName;

        CharacterPrefabConfig* characterPrefabConfig;

        // UI data
        CharacterConfigUIModel characterConfig;

        std::vector<std::string> registeredClassNames;
        std::vector<std::string> modelNames;
        std::vector<std::string> skeletonNames;
        std::vector<std::string> statemachineNames;

    };
}
