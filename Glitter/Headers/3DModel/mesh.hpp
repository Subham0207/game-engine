#pragma once 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <string>
#include <vector>

#include "assimp/scene.h"

#include "Helpers/shader.hpp"
#include "3DModel/Skeleton/Bone.hpp"

#include "serializer.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/access.hpp"

#include <Modals/vertex.hpp>
#include <Modals/texture.hpp>

    class Mesh {
    public:
        // mesh data
        std::vector<ProjectModals::Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<ProjectModals::Texture> *textureIds;

        Mesh()=default;
        Mesh(std::vector<ProjectModals::Vertex> vertices, std::vector<unsigned int> indices, std::vector<ProjectModals::Texture> *textureIds);
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
