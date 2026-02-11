#include <Character/Character.hpp>

#include "Character/CharacterPrefabConfig.hpp"
#include "Shared/EditableText.hpp"

namespace UI
{
    struct CharacterConfigUIModel
    {
        int selectedRegisteredCharacterIndex;

        int selectedModelIndex;

        glm::vec3 modelRelativePosition;
        float capsuleHalfHeight;
        float capsuleRadius;
        glm::vec3 modelScale;

        int selectedSkeletonIndex;
        int selectedStateMachineIndex;
        int selectedPlayerControllerIndex;
    };

    class CharacterUI
    {
    public:
        CharacterUI();
        void draw();
        void start(CharacterPrefabConfig& characterPrefab, std::string characterMetaFilePath = "");
        bool showCharacterUI;
    private:
        int toUiIndex(int dataTypeIndex);
        int toDataTypeIndex(int UiIndex);

        Shared::EditableText characterName;

        CharacterPrefabConfig* characterPrefabConfig;

        // UI data
        CharacterConfigUIModel characterConfigUIModel;

        std::vector<std::string> registeredClassNames;
        std::vector<std::string> modelNames;
        std::vector<std::string> skeletonNames;
        std::vector<std::string> statemachineNames;
        std::vector<std::string> playerControllerNames;

    };
}
