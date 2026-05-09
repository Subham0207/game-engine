#include "../Headers/NodeGraph/FlowScript/Compile/NodePositionSerialization.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace Flowscript::Compile
{
    std::string NodePositionSerialization::Serialize(const std::vector<NodePosition>& positions)
    {
        std::ostringstream out;

        for (size_t i = 0; i < positions.size(); ++i)
        {
            if (i > 0)
                out << ";";

            out << std::fixed << std::setprecision(2) << positions[i].x << "," << positions[i].y;
        }

        return out.str();
    }

    std::vector<NodePosition> NodePositionSerialization::Deserialize(const std::string& serialized)
    {
        std::vector<NodePosition> positions;
        if (serialized.empty())
            return positions;

        std::istringstream stream(serialized);
        std::string token;
        while (std::getline(stream, token, ';'))
        {
            if (token.empty())
                continue;

            const size_t comma = token.find(',');
            if (comma == std::string::npos)
                throw std::runtime_error("Invalid node position token: " + token);

            const std::string xPart = token.substr(0, comma);
            const std::string yPart = token.substr(comma + 1);
            if (xPart.empty() || yPart.empty())
                throw std::runtime_error("Invalid node position token: " + token);

            try
            {
                NodePosition pos;
                pos.x = std::stof(xPart);
                pos.y = std::stof(yPart);
                positions.push_back(pos);
            }
            catch (const std::exception&)
            {
                throw std::runtime_error("Invalid node position token: " + token);
            }
        }

        return positions;
    }
}

