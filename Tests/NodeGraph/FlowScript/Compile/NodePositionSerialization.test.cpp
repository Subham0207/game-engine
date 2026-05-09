#include <gtest/gtest.h>
#include <stdexcept>
#include <NodeGraph/FlowScript/Compile/NodePositionSerialization.hpp>

using Flowscript::Compile::NodePosition;
using Flowscript::Compile::NodePositionSerialization;

TEST(NodePositionSerialization, shouldSerializePositionsWithStableFormatting)
{
    const std::vector<NodePosition> positions{
        {10.0f, 20.0f},
        {30.123f, 40.678f},
        {-5.0f, 0.0f}
    };

    const std::string serialized = NodePositionSerialization::Serialize(positions);
    ASSERT_EQ(serialized, "10.00,20.00;30.12,40.68;-5.00,0.00");
}

TEST(NodePositionSerialization, shouldDeserializeSerializedPositions)
{
    const std::string serialized = "10.00,20.00;30.12,40.68;-5.00,0.00";

    const auto positions = NodePositionSerialization::Deserialize(serialized);
    ASSERT_EQ(positions.size(), 3u);
    EXPECT_FLOAT_EQ(positions[0].x, 10.0f);
    EXPECT_FLOAT_EQ(positions[0].y, 20.0f);
    EXPECT_FLOAT_EQ(positions[1].x, 30.12f);
    EXPECT_FLOAT_EQ(positions[1].y, 40.68f);
    EXPECT_FLOAT_EQ(positions[2].x, -5.0f);
    EXPECT_FLOAT_EQ(positions[2].y, 0.0f);
}

TEST(NodePositionSerialization, shouldThrowOnInvalidToken)
{
    EXPECT_THROW(NodePositionSerialization::Deserialize("10.0,20.0;invalid"), std::runtime_error);
}
