#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "NodeGraph/StateMachineJsonExporter.hpp"

TEST(StateMachineJsonExporter, shouldSerializeTransitionScriptPaths)
{
    StateMachineNode root;
    root.id = 1;
    root.name = "Root";

    StateMachineNode next;
    next.id = 2;
    next.name = "Next";

    StateMachineLink link;
    link.id = 10;
    link.fromNodeId = 1;
    link.toNodeId = 2;
    link.flowScriptPath = "Assets/FlowScripts/sample.flowscript";
    link.luaScriptPath = "Assets/FlowScripts/sample.lua";

    const std::vector<StateMachineNode> nodes{root, next};
    const std::vector<StateMachineLink> links{link};

    const std::string json = StateMachineJsonExporter::ExportChainJson(nodes, links, 1);

    EXPECT_NE(json.find("\"flow_script_path\": \"Assets/FlowScripts/sample.flowscript\""), std::string::npos);
    EXPECT_NE(json.find("\"lua_script_path\": \"Assets/FlowScripts/sample.lua\""), std::string::npos);
}

TEST(StateMachineJsonExporter, shouldDeserializeTransitionScriptPaths)
{
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "StateMachineJsonExporter_shouldDeserializeTransitionScriptPaths.sm";

    const std::string json =
        "{"
        "\"root\":1,"
        "\"nodes\":[{\"id\":1,\"name\":\"Root\",\"pos\":{\"x\":0,\"y\":0},\"type\":2},{\"id\":2,\"name\":\"Next\",\"pos\":{\"x\":0,\"y\":0},\"type\":2}],"
        "\"transitions\":[{\"id\":10,\"from\":1,\"to\":2,\"from_side\":1,\"to_side\":3,\"from_offset_grid\":{\"x\":0,\"y\":0},\"to_offset_grid\":{\"x\":0,\"y\":0},\"flow_script_path\":\"Assets/FlowScripts/sample.flowscript\",\"lua_script_path\":\"Assets/FlowScripts/sample.lua\"}]"
        "}";

    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        out << json;
    }

    std::vector<StateMachineNode> outNodes;
    std::vector<StateMachineLink> outLinks;
    int rootId = -1;

    const bool ok = StateMachineJsonExporter::DeserializeChainJson(tempPath.string(), outNodes, outLinks, rootId);
    EXPECT_TRUE(ok);
    ASSERT_EQ(outLinks.size(), 1u);
    EXPECT_EQ(outLinks[0].flowScriptPath, "Assets/FlowScripts/sample.flowscript");
    EXPECT_EQ(outLinks[0].luaScriptPath, "Assets/FlowScripts/sample.lua");

    std::error_code ec;
    std::filesystem::remove(tempPath, ec);
}


