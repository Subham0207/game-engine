#pragma once
#include <vector>
#include <3DModel/model.hpp>
#include <Renderable/renderable.hpp>

class CapsuleColliderModel: public Renderable{
    public:
        CapsuleColliderModel(float radius = 0.5F, float halfHeight = 1.0F);

        void reGenerateCapsuleColliderMesh(float radius, float halfheight);

        void draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap) override;

        std::vector<Modals::Material *> getMaterials() override;

        void imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix) override
        {
            model->imguizmoManipulate(viewMatrix, projMatrix);
        };
        std::vector<Mesh>* getMeshes() override
        {
            return model->getMeshes();
        };
        glm::mat4& getModelMatrix() override
        {
            return model->getModelMatrix();
        };
        std::string getName() override
        {
            return model->getName();
        };
        ProjectModals::Texture* LoadTexture(std::string filePath, aiTextureType textureType) override
        {
            return model->LoadTexture(filePath, textureType);
        };

        virtual glm::vec3 GetPosition()
        {
            return model->GetPosition();
        }

        virtual glm::vec3 GetScale()
        {
            return model->GetScale();
        }

        virtual glm::quat GetRot()
        {
            return model->GetRot();
        }

        virtual std::string GetGuid() override {
            return "random_guid";
        }

        bool ShouldRender() override;

        Model* model;

        void GenerateCapsuleMesh(std::vector<ProjectModals::Vertex>& vertices, std::vector<unsigned int>& indices, float radius, float halfHeight, int segments, int rings);

        Model* createCapsuleModel(float radius = 0.5f, float halfHeight = 1.0f, int segments = 16, int rings = 8);
};