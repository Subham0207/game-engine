#pragma once
#include <imgui.h>
#include <GraphEditor.h>
#include <vector>

namespace UI::StateMachineGraph{

    struct GraphEditorDelegate : public GraphEditor::Delegate
    {
        GraphEditorDelegate()
        {
            mTemplates = std::vector<GraphEditor::Template>();
            mNodes = std::vector<Node>();
            mLinks = std::vector<GraphEditor::Link>();
        }

        bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override
        {
            return true;
        }

        void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override
        {
            mNodes[nodeIndex].mSelected = selected;
        }

        void MoveSelectedNodes(const ImVec2 delta) override
        {
            for (auto& node : mNodes)
            {
                if (!node.mSelected)
                {
                    continue;
                }
                node.x += delta.x;
                node.y += delta.y;
            }
        }

        virtual void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override
        {
        }

        void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override
        {
            mLinks.push_back({ inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex });
        }

        void DelLink(GraphEditor::LinkIndex linkIndex) override
        {
            mLinks.erase(mLinks.begin() + linkIndex);
        }

        void CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override
        {
            drawList->AddLine(rectangle.Min, rectangle.Max, IM_COL32(0, 0, 0, 255));
            drawList->AddText(rectangle.Min, IM_COL32(255, 128, 64, 255), "Draw");
        }

        const size_t GetTemplateCount() override
        {
            return sizeof(mTemplates) / sizeof(GraphEditor::Template);
        }

        const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override
        {
            return mTemplates[index];
        }

        const size_t GetNodeCount() override
        {
            return mNodes.size();
        }

        const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override
        {
            const auto& myNode = mNodes[index];
            return GraphEditor::Node
            {
                myNode.name,
                myNode.templateIndex,
                ImRect(ImVec2(myNode.x, myNode.y), ImVec2(myNode.x + 200, myNode.y + 200)),
                myNode.mSelected
            };
        }

        const size_t GetLinkCount() override
        {
            return mLinks.size();
        }

        const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override
        {
            return mLinks[index];
        }

        void clear()
        {
            mNodes.clear();
            mTemplates.clear();
            mLinks.clear();
        }

        // Graph datas
        /*
            header color,
            backgroud color,
            background color over,

            input count,
            input names,
            input colors,

            output count,
            output names,
            outputcolor 
        */
        std::vector<GraphEditor::Template> mTemplates;

        struct Node
        {
            const char* name;
            GraphEditor::TemplateIndex templateIndex;
            float x, y;
            bool mSelected;
        };

        /*
            node name,
            template index -- to apply that template -- this is where we pass values
            position x, positio y,
            is not selected,
        */
        std::vector<Node> mNodes;

        // Data for each node
        // left links are inputs and right links are outputs
        // { inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex }
        // input slot index on output node; output slot index on input node; connection goes from input to ouput node. 
        std::vector<GraphEditor::Link> mLinks;
    };

}