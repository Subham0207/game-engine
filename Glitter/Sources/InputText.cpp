#include <UI/Shared/InputText.hpp>

bool UI::Shared::InputText(const char *label, std::string &str, ImGuiInputTextFlags flags)
{
    // Ensure the buffer is large enough to hold the text
    char buffer[256];
    std::strncpy(buffer, str.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    // Create the InputText widget
    bool result = ImGui::InputText(label, buffer, sizeof(buffer), flags);

    // Update the std::string if the text was modified
    if (result)
    {
        str = std::string(buffer);
    }

    return result;
}