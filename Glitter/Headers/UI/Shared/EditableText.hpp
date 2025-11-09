#include <string>

namespace UI::Shared{

    struct EditableText {
        std::string value;   // committed value
        std::string draft;   // edit buffer while in edit mode
        bool editing = false;
    };
    
    bool EditableTextUI(const char* label, EditableText &text);
}