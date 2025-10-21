#include <Character/Character.hpp>

namespace UI
{
    class CharacterUI
    {
        public:
        static void draw(Character* character,  bool &showUI);
            int selectedStateMachineIndex;
            bool showCharacterUI;
            Character* UIOpenedForCharacter;
    };
}