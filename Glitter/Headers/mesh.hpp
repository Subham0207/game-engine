#pragma once 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

#include <assimp/scene.h>

#include <shader.hpp>
#include <Bone.hpp>

#include <serializer.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>

#define MAX_BONE_INFLUENCE 4

    struct Vertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
        // vertex color
        glm::vec4 Color;
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];

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

    struct Texture {
    unsigned int id;
    aiTextureType type;
    std::string name;

    Texture()=default;
    Texture(unsigned int textureId, aiTextureType textureType, std::string filename)
        : id(textureId), type(textureType), name(filename) {}

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & id;
            ar & type;
            ar & name;
        }
    };

    class Mesh {
    public:
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> *textureIds;

        Mesh()=default;
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> *textureIds);
        void Draw(Shader* shader);
        void setupMesh();
    private:
        //  render data
        unsigned int VAO, VBO, EBO;


        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & vertices;
            ar & indices;
        }
    };
