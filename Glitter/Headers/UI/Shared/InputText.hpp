#pragma once
#include <string>
#include <imgui.h>

namespace UI::Shared{

    bool InputText(const char *label, std::string &str, ImGuiInputTextFlags flags = 0);
}
