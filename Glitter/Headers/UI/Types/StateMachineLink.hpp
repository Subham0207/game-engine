#pragma once
#include <string>

struct SMLink {
    int id;           // unique
    int fromNode;     // node id
    int fromPin;      // output pin idx
    int toNode;       // node id
    int toPin;        // input pin idx
    std::string cond; // your condition text/Lua id
};
