#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Shared/EditableText.hpp"

class Character;

namespace UI
{
    class AI_UI
    {
        public:
            AI_UI();
            static void start();
            void draw();
            bool showUI;
        private:
            void save() const;

            Shared::EditableText filename;
            std::vector<std::string> charactersList;
            std::vector<std::shared_ptr<Character>> characters;
            int selectedCharacterFromList;
    };
}