#pragma once

#include <memory>
#include <vector>

#include "NodeGraph/FlowScript/Decompile/ParsedStatement.hpp"

class NodeGraphNode;
class NodeGraphNodeLink;
class NodeGraphNodesView;

namespace Flowscript::Decompile
{
    class DecompileGraphBuilder
    {
    public:
        DecompileGraphBuilder(std::vector<std::unique_ptr<NodeGraphNode>>& allNodes,
                              std::vector<NodeGraphNodeLink>& allLinks,
                              NodeGraphNodesView& graphNodeView);
        ~DecompileGraphBuilder();

        void apply(const ParsedStatement& statement);
        void finalize();

    private:
        class Impl;
        std::unique_ptr<Impl> impl;
    };
}

