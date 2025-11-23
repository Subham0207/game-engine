#include <Lights/light.hpp>
#include <3DModel/model.hpp>
#include <EngineState.hpp>
#include <filesystem>
#include <Modals/3DModelType.hpp>

BaseLight::BaseLight(LightType lightType, glm::vec3 pos)
{
    auto loc = std::filesystem::path(EngineState::state->engineInstalledDirctory);

    switch (lightType)
    {
        case LightType::Directional :
        {
            lightModel = new Model((loc / "EngineAssets" / "cube.fbx").string());
            lightModel->setDirName("directionalLight");
            break;
        }
        case LightType::Point :
        {
            
            lightModel = new Model((loc / "EngineAssets" / "cube.fbx").string());
            lightModel->setDirName("pointLight");
            break;
        }
        case LightType::Spot :
        {
            
            lightModel = new Model((loc / "EngineAssets" / "cube.fbx").string());
            lightModel->setDirName("spotLight");
            break;			
        }
        default:
            break;

    }

    lightModel->modeltype = ModelType::LIGHT;
    lightModel->setTransform(pos,glm::quat(),glm::vec3(0.03f));
    getActiveLevel().addRenderable(lightModel);
}