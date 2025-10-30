#pragma once
#include <imgui.h>
#include <string>

struct SMNode {
    int id;               
    std::string guid;     
    std::string label;    
    int type;              
    ImVec2 pos{100,100};
    ImVec2 size{160,80};
};