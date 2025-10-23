#include <Character/Character.hpp>

namespace UI
{
    class CharacterUI
    {
        public:
        CharacterUI()
        {
            selectedStateMachineIndex = 0;
            showCharacterUI = false;
            UIOpenedForCharacter = nullptr;
        }
        static void draw(Character* character,  bool &showUI);
            int selectedStateMachineIndex;
            bool showCharacterUI;
            Character* UIOpenedForCharacter;
    };
}