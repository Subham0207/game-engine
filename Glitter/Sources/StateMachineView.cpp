//
// Created by subha on 26-03-2026.
//

#include <NodeGraph/Views/StateMachineView.hpp>
#include <NodeGraph/FlowScript/StatemachineFlowScript.hpp>
#include <EngineState.hpp>

#include <filesystem>

namespace
{
    std::string displayNameFromPath(const std::string& path)
    {
        const std::string name = std::filesystem::path(path).filename().stem().string();
        return name.empty() ? path : name;
    }
}

void StateMachineView::refreshAvailableResources()
{
    animations.animationguids.clear();
    animations.animationnamesStorage.clear();
    animations.animationnames.clear();

    blendspaces.blendspaceguids.clear();
    blendspaces.blendspacenamesStorage.clear();
    blendspaces.blendspacenames.clear();

    animations.animationguids.push_back("");
    animations.animationnamesStorage.push_back("None");
    blendspaces.blendspaceguids.push_back("");
    blendspaces.blendspacenamesStorage.push_back("None");

    if (EngineState::state && EngineState::state->engineRegistry)
    {
        for (const auto& kv : EngineState::state->engineRegistry->animationsFileMap)
        {
            animations.animationguids.push_back(kv.first);
            animations.animationnamesStorage.push_back(displayNameFromPath(kv.second));
        }

        for (const auto& kv : EngineState::state->engineRegistry->blendpaceFileMap)
        {
            blendspaces.blendspaceguids.push_back(kv.first);
            blendspaces.blendspacenamesStorage.push_back(displayNameFromPath(kv.second));
        }
    }

    animations.animationnames.reserve(animations.animationnamesStorage.size());
    for (const std::string& name : animations.animationnamesStorage)
        animations.animationnames.push_back(name.c_str());

    blendspaces.blendspacenames.reserve(blendspaces.blendspacenamesStorage.size());
    for (const std::string& name : blendspaces.blendspacenamesStorage)
        blendspaces.blendspacenames.push_back(name.c_str());
}

int StateMachineView::findGuidIndex(const std::vector<std::string>& guids, const std::string& guid)
{
    for (int i = 0; i < static_cast<int>(guids.size()); ++i)
    {
        if (guids[i] == guid)
            return i;
    }

    return 0;
}

int StateMachineView::findFieldNameIndex(const std::vector<std::string>& fieldNames, const std::string& fieldName)
{
    for (int i = 0; i < static_cast<int>(fieldNames.size()); ++i)
    {
        if (fieldNames[i] == fieldName)
            return i;
    }

    return 0;
}

void StateMachineView::EditCondition(StateMachineLink* selectedLink) const
{
    if (ImGui::Button("Edit Condition"))
    {
        if (mSmflowscriptRef)
        {
            mSmflowscriptRef->setSelectedLink(selectedLink);
        }
    }
}

void StateMachineView::nodeBody(
    std::vector<StateMachineNode>& nodes,
    StateMachineNode& currentNode
    )
{
    if (ImGui::SmallButton((std::string("Make Active##") + std::to_string(currentNode.id)).c_str()))
        setActive(nodes, currentNode.id);

    ImGui::PushID(currentNode.id);

    const char* typeLabels[] = {"None", "Blendspace", "Animation"};
    int typeIndex = 0;
    if (currentNode.type == StateMachineNodeType::Blendspace)
        typeIndex = 1;
    else if (currentNode.type == StateMachineNodeType::Animation)
        typeIndex = 2;

    ImGui::SetNextItemWidth(180.0f);
    if (ImGui::Combo("Type", &typeIndex, typeLabels, IM_ARRAYSIZE(typeLabels)))
    {
        StateMachineNodeType selectedType = StateMachineNodeType::None;
        if (typeIndex == 1)
            selectedType = StateMachineNodeType::Blendspace;
        else if (typeIndex == 2)
            selectedType = StateMachineNodeType::Animation;

        currentNode.type = selectedType;

        if (currentNode.type == StateMachineNodeType::None)
        {
            currentNode.resourceGuid.clear();
            currentNode.blendspaceAxisXField.clear();
            currentNode.blendspaceAxisYField.clear();
        }
        else
        {
            const auto& guids =
                (currentNode.type == StateMachineNodeType::Blendspace)
                    ? blendspaces.blendspaceguids
                    : animations.animationguids;

            if (findGuidIndex(guids, currentNode.resourceGuid) == 0)
                currentNode.resourceGuid.clear();

            if (currentNode.type != StateMachineNodeType::Blendspace)
            {
                currentNode.blendspaceAxisXField.clear();
                currentNode.blendspaceAxisYField.clear();
            }
        }
    }

    if (currentNode.type == StateMachineNodeType::Blendspace)
    {
        if (contextFloatFields.fieldNamesStorage.empty())
        {
            contextFloatFields.fieldNamesStorage.push_back("None");
            contextFloatFields.fieldNames.push_back(contextFloatFields.fieldNamesStorage.back().c_str());
        }

        int selectedIndex = findGuidIndex(blendspaces.blendspaceguids, currentNode.resourceGuid);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::Combo("Blendspace", &selectedIndex, blendspaces.blendspacenames.data(), static_cast<int>(blendspaces.blendspacenames.size())))
            currentNode.resourceGuid = blendspaces.blendspaceguids[selectedIndex];

        int axisXIndex = findFieldNameIndex(contextFloatFields.fieldNamesStorage, currentNode.blendspaceAxisXField);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::Combo("Axis X", &axisXIndex, contextFloatFields.fieldNames.data(), static_cast<int>(contextFloatFields.fieldNames.size())))
        {
            currentNode.blendspaceAxisXField = (axisXIndex == 0)
                ? ""
                : contextFloatFields.fieldNamesStorage[axisXIndex];
        }

        int axisYIndex = findFieldNameIndex(contextFloatFields.fieldNamesStorage, currentNode.blendspaceAxisYField);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::Combo("Axis Y", &axisYIndex, contextFloatFields.fieldNames.data(), static_cast<int>(contextFloatFields.fieldNames.size())))
        {
            currentNode.blendspaceAxisYField = (axisYIndex == 0)
                ? ""
                : contextFloatFields.fieldNamesStorage[axisYIndex];
        }
    }
    else if (currentNode.type == StateMachineNodeType::Animation)
    {
        int selectedIndex = findGuidIndex(animations.animationguids, currentNode.resourceGuid);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::Combo("Animation", &selectedIndex, animations.animationnames.data(), static_cast<int>(animations.animationnames.size())))
            currentNode.resourceGuid = animations.animationguids[selectedIndex];
    }

    ImGui::PopID();

}
