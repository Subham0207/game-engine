#include <Character/Character.hpp>

namespace UI
{
    class CharacterUI
    {
    public:
        CharacterUI();
        void draw(bool &showUI);
        bool showCharacterUI;
    private:
        std::string characterName;
    };
}