#include <Lights/light.hpp>
#include <3DModel/model.hpp>
#include <EngineState.hpp>
#include <filesystem>

BaseLight::BaseLight(LightType lightType, glm::vec3 pos)
{
    auto loc = std::filesystem::path(EngineState::state->engineInstalledDirctory);

    switch (lightType)
    {
        case LightType::Directional :
        {
            lightModel = new Model((loc / "EngineAssets" / "cube.fbx").string());
            break;
        }
        case LightType::Point :
        {
            
            lightModel = new Model((loc / "EngineAssets" / "cube.fbx").string());
            break;
        }
        case LightType::Spot :
        {
            
            lightModel = new Model((loc / "EngineAssets" / "cube.fbx").string());
            break;			
        }
        default:
            break;

    }


    lightModel->setDirName("light");
    lightModel->setTransform(pos,glm::quat(),glm::vec3(0.03f));
    getActiveLevel().addRenderable(lightModel);
}