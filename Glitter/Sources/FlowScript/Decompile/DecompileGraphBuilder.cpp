#include "NodeGraph/FlowScript/Decompile/DecompileGraphBuilder.hpp"
#include "NodeGraph/FlowScript/Decompile/Helpers.hpp"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <unordered_map>

#include "NodeGraph/Components/NodeGraphNodes/NodeTypes.hpp"
#include "NodeGraph/Views/NodeGraphNodesView.hpp"

namespace
{
    struct NodeOutputRef
    {
        NodeGraphNode* node = nullptr;
        int outputAttr = -1;
    };

    struct PendingGenericObject
    {
        std::vector<NodeGraphComponents::Node::GenericMemberSpec> members;
        std::unordered_map<std::string, size_t> fieldToMemberIndex;
    };

    struct FunctionScope
    {
        NodeGraphNode* functionNode = nullptr;
        std::vector<NodeGraphNode*> execChain;
    };

}

namespace Flowscript::Decompile
{
    class DecompileGraphBuilder::Impl
    {
    public:
        Impl(std::vector<std::unique_ptr<NodeGraphNode>>& allNodes,
             std::vector<NodeGraphNodeLink>& allLinks,
             NodeGraphNodesView& graphNodeView)
            : nodes(allNodes),
              nodeGraphLinks(allLinks),
              nodeView(graphNodeView)
        {
        }

        void apply(const ParsedStatement& statement)
        {
            switch (statement.kind)
            {
            case ParsedStatementKind::LocalAssignment:
                handleLocalAssignment(statement.local);
                return;
            case ParsedStatementKind::FieldAssignment:
                if (registerGenericField(statement.field))
                    return;
                flushPendingGenericObjects();
                throw std::runtime_error("Unsupported Lua statement: " + statement.field.rawLine);
            case ParsedStatementKind::Print:
                flushPendingGenericObjects();
                appendPrint(statement.print);
                return;
            case ParsedStatementKind::Return:
                flushPendingGenericObjects();
                appendReturn(statement.ret);
                return;
            case ParsedStatementKind::End:
                flushPendingGenericObjects();
                closeFunctionScope();
                return;
            }

            throw std::runtime_error("Unsupported parsed statement kind");
        }

        void finalize()
        {
            flushPendingGenericObjects();

            if (!functionStack.empty())
                throw std::runtime_error("Unclosed function block in Lua");

            if (!topLevelExec.empty())
                linkExecChain(topLevelExec);
        }

    private:
        NodeGraphNode* addNode(NodeTypes type)
        {
            const size_t before = nodes.size();
            nodeView.addNode(nodes, type, nextPos());
            if (nodes.size() <= before)
                throw std::runtime_error("Failed to create node");
            return nodes.back().get();
        }

        NodeGraphNode* addGenericTypeNode(const std::string& objectName,
                                          const std::vector<NodeGraphComponents::Node::GenericMemberSpec>& members)
        {
            const size_t before = nodes.size();
            nodeView.addGenericTypeNode(nodes, objectName, members, nextPos());
            if (nodes.size() <= before)
                throw std::runtime_error("Failed to create generic object node");
            return nodes.back().get();
        }

        ImVec2 nextPos()
        {
            ImVec2 pos(120.0f + kLayoutStepX * static_cast<float>(layoutX),
                       120.0f + kLayoutStepY * static_cast<float>(layoutY));
            layoutX++;
            if (layoutX >= 4)
            {
                layoutX = 0;
                layoutY++;
            }
            return pos;
        }

        static void setFieldValue(NodeGraphNode* node, const std::string& value)
        {
            if (!node || node->outputs().empty())
                throw std::runtime_error("Node has no outputs to set");

            char* buff = node->outputs()[0].getValueBuff();
            std::snprintf(buff, NodeGraphComponents::Node::Attribute::getValueSize(), "%s", value.c_str());
        }

        static int getOutputAttr(NodeGraphNode* node)
        {
            if (!node || node->outputs().empty())
                throw std::runtime_error("Node has no output attribute");
            return node->outputs()[0].getId();
        }

        static std::vector<std::string> parseFunctionParameters(const std::string& expression)
        {
            if (!Helpers::startsWith(expression, "function(") || expression.back() != ')')
                return {};

            const std::string paramsExpr = Helpers::trimCopy(
                expression.substr(std::strlen("function("), expression.size() - std::strlen("function(") - 1));
            if (paramsExpr.empty())
                return {};

            std::vector<std::string> params;
            size_t start = 0;
            while (start < paramsExpr.size())
            {
                const size_t comma = paramsExpr.find(',', start);
                const std::string raw = (comma == std::string::npos)
                    ? paramsExpr.substr(start)
                    : paramsExpr.substr(start, comma - start);
                const std::string trimmed = Helpers::trimCopy(raw);
                if (!trimmed.empty())
                    params.push_back(trimmed);

                if (comma == std::string::npos)
                    break;
                start = comma + 1;
            }

            return params;
        }

        NodeOutputRef resolveValue(const std::string& rawValue)
        {
            const std::string value = Helpers::trimCopy(rawValue);
            const auto existingIt = vars.find(value);
            if (existingIt != vars.end() && existingIt->second.node && existingIt->second.outputAttr >= 0)
                return existingIt->second;

            if (Helpers::startsWith(value, "node_"))
                throw std::runtime_error("Unknown variable reference: " + value);

            if (Helpers::isBooleanLiteral(value))
            {
                auto* node = addNode(NodeTypes::Boolean);
                setFieldValue(node, value);
                return { node, getOutputAttr(node) };
            }

            if (Helpers::isNumberLiteral(value))
            {
                auto* node = addNode(NodeTypes::Integer);
                setFieldValue(node, value);
                return { node, getOutputAttr(node) };
            }

            throw std::runtime_error("Unsupported literal: " + value);
        }

        void linkDataInputs(NodeGraphNode* node, const std::vector<std::string>& inputs)
        {
            if (!node)
                return;

            auto& nodeInputs = node->inputs();
            if (nodeInputs.size() < inputs.size())
                throw std::runtime_error("Node input arity mismatch");

            for (size_t i = 0; i < inputs.size(); ++i)
            {
                const auto ref = resolveValue(inputs[i]);
                nodeView.addLink(nodeGraphLinks, ref.outputAttr, nodeInputs[i].getId());
            }
        }

        void linkExecChain(const std::vector<NodeGraphNode*>& chain)
        {
            NodeGraphNode* prev = nullptr;
            for (auto* node : chain)
            {
                if (!node)
                    continue;

                NodeAttribute* in = node->getExecInput();
                NodeAttribute* out = node->getExecOutput();
                if (prev && in && prev->getExecOutput())
                    nodeView.addLink(nodeGraphLinks, prev->getExecOutput()->getId(), in->getId());

                prev = out ? node : nullptr;
            }
        }

        void appendExecNode(NodeGraphNode* node)
        {
            if (!node)
                return;

            if (functionStack.empty())
                topLevelExec.push_back(node);
            else
                functionStack.back().execChain.push_back(node);
        }

        bool parseBinaryExpression(const std::string& expr,
                                   NodeTypes& nodeType,
                                   std::string& lhs,
                                   std::string& rhs,
                                   bool& shouldJoinExecChain) const
        {
            if (expr.size() < 2 || expr.front() != '(' || expr.back() != ')')
                return false;

            const std::string inner = Helpers::trimCopy(expr.substr(1, expr.size() - 2));

            if (inner.find(" + ") != std::string::npos)
            {
                auto parts = Helpers::splitOnce(inner, " + ");
                nodeType = NodeTypes::Add;
                lhs = parts.first;
                rhs = parts.second;
                shouldJoinExecChain = true;
                return true;
            }
            if (inner.find(" - ") != std::string::npos)
            {
                auto parts = Helpers::splitOnce(inner, " - ");
                nodeType = NodeTypes::Subtract;
                lhs = parts.first;
                rhs = parts.second;
                shouldJoinExecChain = true;
                return true;
            }
            if (inner.find(" > ") != std::string::npos)
            {
                auto parts = Helpers::splitOnce(inner, " > ");
                nodeType = NodeTypes::GreaterThan;
                lhs = parts.first;
                rhs = parts.second;
                shouldJoinExecChain = true;
                return true;
            }
            if (inner.find(" == ") != std::string::npos)
            {
                auto parts = Helpers::splitOnce(inner, " == ");
                nodeType = NodeTypes::EqualsTo;
                lhs = parts.first;
                rhs = parts.second;
                shouldJoinExecChain = false;
                return true;
            }
            if (inner.find(" ~= ") != std::string::npos)
            {
                auto parts = Helpers::splitOnce(inner, " ~= ");
                nodeType = NodeTypes::NotEqualsTo;
                lhs = parts.first;
                rhs = parts.second;
                shouldJoinExecChain = false;
                return true;
            }

            throw std::runtime_error("Unsupported expression: " + expr);
        }

        void handleLocalAssignment(const ParsedLocalAssignment& assignment)
        {
            const std::string& varName = assignment.variableName;
            const std::string& expr = assignment.expression;

            if (expr == "{}")
            {
                if (pendingGenericObjects.find(varName) == pendingGenericObjects.end())
                    pendingGenericOrder.push_back(varName);
                pendingGenericObjects[varName];
                return;
            }

            flushPendingGenericObjects();

            if (Helpers::startsWith(expr, "function(") && expr.back() == ')')
            {
                const auto parameters = parseFunctionParameters(expr);
                auto* functionNode = nodeView.addFunctionNode(nodes, parameters, nextPos());
                vars[varName] = { functionNode, -1 };
                functionStack.push_back({ functionNode, {} });
                return;
            }

            NodeTypes nodeType = NodeTypes::Add;
            std::string lhs;
            std::string rhs;
            bool shouldJoinExecChain = false;
            if (parseBinaryExpression(expr, nodeType, lhs, rhs, shouldJoinExecChain))
            {
                auto* node = addNode(nodeType);
                vars[varName] = { node, getOutputAttr(node) };
                linkDataInputs(node, { lhs, rhs });
                if (shouldJoinExecChain)
                    appendExecNode(node);
                return;
            }

            if (Helpers::isBooleanLiteral(expr))
            {
                auto* node = addNode(NodeTypes::Boolean);
                setFieldValue(node, expr);
                vars[varName] = { node, getOutputAttr(node) };
                return;
            }

            if (Helpers::isNumberLiteral(expr))
            {
                auto* node = addNode(NodeTypes::Integer);
                setFieldValue(node, expr);
                vars[varName] = { node, getOutputAttr(node) };
                return;
            }

            throw std::runtime_error("Unsupported assignment: " + assignment.rawLine);
        }

        bool registerGenericField(const ParsedFieldAssignment& field)
        {
            auto objectIt = pendingGenericObjects.find(field.objectName);
            if (objectIt == pendingGenericObjects.end())
                return false;

            const bool isBool = Helpers::isBooleanLiteral(field.value);
            const bool isNumber = Helpers::isNumberLiteral(field.value);
            if (!isBool && !isNumber)
                return true; // Ignore unsupported literals for GenericType v1.

            auto& pending = objectIt->second;
            auto memberIt = pending.fieldToMemberIndex.find(field.fieldName);
            if (memberIt == pending.fieldToMemberIndex.end())
            {
                NodeGraphComponents::Node::GenericMemberSpec spec;
                spec.name = field.fieldName;
                spec.literalValue = field.value;
                spec.isBoolean = isBool;

                pending.fieldToMemberIndex[field.fieldName] = pending.members.size();
                pending.members.push_back(std::move(spec));
            }
            else
            {
                auto& spec = pending.members[memberIt->second];
                spec.literalValue = field.value;
                spec.isBoolean = isBool;
            }

            return true;
        }

        void flushPendingGenericObjects()
        {
            for (const auto& objectName : pendingGenericOrder)
            {
                const auto objectIt = pendingGenericObjects.find(objectName);
                if (objectIt == pendingGenericObjects.end())
                    continue;

                const auto& members = objectIt->second.members;
                if (members.empty())
                    continue;

                auto* genericNode = addGenericTypeNode(objectName, members);
                auto& outputs = genericNode->outputs();
                const size_t count = std::min(outputs.size(), members.size());
                for (size_t i = 0; i < count; ++i)
                    vars[objectName + "." + members[i].name] = { genericNode, outputs[i].getId() };
            }

            pendingGenericObjects.clear();
            pendingGenericOrder.clear();
        }

        void appendPrint(const ParsedPrintStatement& printStatement)
        {
            auto* node = addNode(NodeTypes::Print);
            linkDataInputs(node, { printStatement.expression });
            appendExecNode(node);
        }

        void appendReturn(const ParsedReturnStatement& returnStatement)
        {
            if (!returnStatement.expression.empty() && returnStatement.expression.front() == '{')
                throw std::runtime_error("Return of table not supported");

            auto* node = addNode(NodeTypes::Return);
            linkDataInputs(node, { returnStatement.expression });
            appendExecNode(node);
        }

        void closeFunctionScope()
        {
            if (functionStack.empty())
                throw std::runtime_error("Unexpected 'end' without function");

            auto scope = functionStack.back();
            functionStack.pop_back();

            if (!scope.execChain.empty())
            {
                auto* first = scope.execChain.front();
                if (scope.functionNode && scope.functionNode->getExecOutput() && first->getExecInput())
                    nodeView.addLink(nodeGraphLinks, scope.functionNode->getExecOutput()->getId(), first->getExecInput()->getId());
                linkExecChain(scope.execChain);
            }
        }

    private:
        std::vector<std::unique_ptr<NodeGraphNode>>& nodes;
        std::vector<NodeGraphNodeLink>& nodeGraphLinks;
        NodeGraphNodesView& nodeView;
        std::unordered_map<std::string, NodeOutputRef> vars;
        std::vector<NodeGraphNode*> topLevelExec;
        std::vector<FunctionScope> functionStack;
        std::unordered_map<std::string, PendingGenericObject> pendingGenericObjects;
        std::vector<std::string> pendingGenericOrder;

        int layoutX = 0;
        int layoutY = 0;
        static constexpr float kLayoutStepX = 220.0f;
        static constexpr float kLayoutStepY = 140.0f;
    };

    DecompileGraphBuilder::DecompileGraphBuilder(std::vector<std::unique_ptr<NodeGraphNode>>& allNodes,
                                                 std::vector<NodeGraphNodeLink>& allLinks,
                                                 NodeGraphNodesView& graphNodeView)
        : impl(std::make_unique<Impl>(allNodes, allLinks, graphNodeView))
    {
    }

    DecompileGraphBuilder::~DecompileGraphBuilder() = default;

    void DecompileGraphBuilder::apply(const ParsedStatement& statement)
    {
        impl->apply(statement);
    }

    void DecompileGraphBuilder::finalize()
    {
        impl->finalize();
    }
}



