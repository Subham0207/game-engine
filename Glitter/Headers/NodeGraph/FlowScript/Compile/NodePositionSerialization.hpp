#ifndef GLITTER_NODEPOSITIONSERIALIZATION_HPP
#define GLITTER_NODEPOSITIONSERIALIZATION_HPP

#include <string>
#include <vector>

namespace Flowscript::Compile
{
    struct NodePosition
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    class NodePositionSerialization
    {
    public:
        static std::string Serialize(const std::vector<NodePosition>& positions);
        static std::vector<NodePosition> Deserialize(const std::string& serialized);
    };
}

#endif //GLITTER_NODEPOSITIONSERIALIZATION_HPP

