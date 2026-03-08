//
// Created by subha on 08-03-2026.
//

#include "../Headers/Modals/vertex.hpp"

ProjectModals::Vertex::Vertex():
Position{0.0f},
Normal{0.0f},
TexCoords{0.0f},
Color{1.0f},
Tangent{0.0f},
Bitangent{0.0f}
{
    std::fill(std::begin(m_BoneIDs), std::end(m_BoneIDs), -1);
    std::fill(std::begin(m_Weights), std::end(m_Weights), 0.0f);
}