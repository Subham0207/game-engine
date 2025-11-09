#include <UI/Shared/EditableText.hpp>
#include <imgui.h>
#include <UI/Shared/InputText.hpp>

void UI::Shared::EditableTextUI(const char* label, EditableText &text)
{
    if(text.editing)
    {
        UI::Shared::InputText(label, text.draft);
        ImGui::SameLine();
        if(ImGui::Button("ok"))
        {
            text.editing = false;
            text.value = text.draft;
        }
        ImGui::SameLine();
        if(ImGui::Button("close"))
        {
            text.editing = false;
        }
    }
    else
    {
        ImGui::Text(text.value.c_str());
        ImGui::SameLine();
        if(ImGui::Button("Edit"))
        {
            text.editing = true;
        }
    }
}