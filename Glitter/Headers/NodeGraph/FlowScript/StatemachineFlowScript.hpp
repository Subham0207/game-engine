//
// Created by subha on 26-03-2026.
//

#ifndef GLITTER_STATEMACHINEFLOWSCRIPT_HPP
#define GLITTER_STATEMACHINEFLOWSCRIPT_HPP
#include "FlowScript.hpp"

class StatemachineFlowScript: public FlowScript
{
public:
    StatemachineFlowScript();
    ~StatemachineFlowScript() override = default;

    void draw();

    void setSelectedLink(StateMachineLink* link)
    {
        clearScript();
        selectedLink = link;
        showUI = true;

        if (selectedLink && !selectedLink->condition.empty())
        {
            setCompiledLua(selectedLink->condition);
            deCompile(selectedLink->condition);
        }
    }

    void close()
    {
        showUI = false;
        selectedLink = nullptr;
    }

    const std::string& compile() override;

private:
    bool showUI;
    StateMachineLink* selectedLink;
};


#endif //GLITTER_STATEMACHINEFLOWSCRIPT_HPP