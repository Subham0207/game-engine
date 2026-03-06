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
        glm::vec3 Position{0.0f};
        // normal
        glm::vec3 Normal{0.0f};
        // texCoords
        glm::vec2 TexCoords{0.0f};
        // vertex color
        glm::vec4 Color{1.0f};
        // tangent
        glm::vec3 Tangent{0.0f};
        // bitangent
        glm::vec3 Bitangent{0.0f};
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE]{};
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE]{};
        
        //Animated vertex pos
        //NOTE: the vertex position is of default pose/T-Pose. Animation is applying on GPU side.
        //Here we are applying the animation and then the below is the updated vertex position which is stored on CPU for selection.
        glm::vec3 animatedPos{0.0f};

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & Position;
            ar & Normal;
            ar & TexCoords;
            ar & Color;
            ar & boost::serialization::make_array(m_BoneIDs, MAX_BONE_INFLUENCE);
            ar & boost::serialization::make_array(m_Weights, MAX_BONE_INFLUENCE);
            ar & Tangent;
            ar & Bitangent;
        }
    };
}
