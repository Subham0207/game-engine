#pragma once
#include <glm/glm.hpp>
#include "serializer.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/access.hpp"

#define MAX_BONE_INFLUENCE 4

namespace ProjectModals{

    struct Vertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
        // vertex color
        glm::vec4 Color;
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & Position;
            ar & Normal;
            ar & TexCoords;
            ar & Color;
            ar & Tangent;
            ar & Bitangent;
            ar & m_BoneIDs;
            ar & m_Weights;
        }
    };
}
