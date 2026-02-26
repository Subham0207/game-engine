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
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>

#include <Modals/vertex.hpp>
#include "Materials/IMaterial.hpp"
#include "Materials/MaterialInstance.hpp"

enum ModelType;
class Camera;
class Lights;
class Mesh {
    public:
        // mesh data
        std::vector<ProjectModals::Vertex>       vertices;
        std::vector<unsigned int> indices;

        std::shared_ptr<Materials::IMaterial> mMaterial;

        Mesh()=default;
        Mesh(std::vector<ProjectModals::Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Materials::IMaterial> material);
        void DrawOnlyGeometry();
        void Draw(Camera* camera, Lights* lightSystem, ModelType modelType, glm::mat4 modelMatrix);
        void setupMesh();

        std::vector<ProjectModals::Vertex> GetWorldVertices();
        std::vector<unsigned int> GetIndices();
    private:
        //  render data
        unsigned int VAO, VBO, EBO;


        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive &ar, const unsigned int version) {
            ar & vertices;
            ar & indices;
            ar & mMaterial;
        }
    };
