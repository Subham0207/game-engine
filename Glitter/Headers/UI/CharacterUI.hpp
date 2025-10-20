#include <Character/Character.hpp>

namespace UI
{
    class CharacterUI
    {
        public:
        static void draw(Character* character);
            int selectedStateMachineIndex;
            bool showCharacterUI;
    };
}