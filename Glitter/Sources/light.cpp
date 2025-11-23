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

DirectionalLight::DirectionalLight(
    glm::vec3 position, // Note we take position to update the light 3d model but for directional light position does not matter.
    glm::vec3 direction,
    glm::vec3 lightColor,
    glm::vec3 diffuseColor,
    glm::vec3 ambientColor,
    glm::vec3 specularColo): BaseLight(LightType::Directional, position)
{
    this->direction = direction;
    this->diffuseColor = lightColor * diffuseColor;
    this->ambientColor = lightColor * diffuseColor * ambientColor;
    this->specularColor = specularColor;
    this->intensity = 100.0f;
}

void DirectionalLight::attachShaderUniforms(
    GLuint shaderId,
    std::string directionUniform,
    std::string colorUniform,
    std::string intensityUniform)
{
    glUniform3f(glGetUniformLocation(shaderId, directionUniform.c_str()), direction.x, direction.y, direction.z);
    glUniform3f(glGetUniformLocation(shaderId, colorUniform.c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
    glUniform1f(glGetUniformLocation(shaderId, intensityUniform.c_str()), intensity);
}

SpotLight::SpotLight(
    glm::vec3 position,
    glm::vec3 lightColor,
    glm::vec3 direction,
    float innerCutOffRadius,
    float outerCutOffRadius,
    glm::vec3 diffuseColor,
    glm::vec3 ambientColor,
    glm::vec3 specularColor): BaseLight(LightType::Spot, position)
{
    this->position = position;
    this->direction = direction;
    this->innerCutOffRadius = innerCutOffRadius;
    this->outerCutOffRadius = outerCutOffRadius;
    this->diffuseColor = lightColor * diffuseColor;
    this->ambientColor = lightColor * diffuseColor * ambientColor;
    this->specularColor = specularColor;
}

void SpotLight::attachShaderUniforms(
    GLuint shaderId,
    std::string positionUniform,
    std::string directionUniform,
    std::string colorUniform,
    std::string intensityUninform,
    std::string innercCutOffUniform,
    std::string outerCutOffUniform
)
{
    glUniform3f(glGetUniformLocation(shaderId, positionUniform.c_str()), position.x, position.y, position.z);
    glUniform3f(glGetUniformLocation(shaderId, directionUniform.c_str()), direction.x, direction.y, direction.z);
    glUniform3f(glGetUniformLocation(shaderId, colorUniform.c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
    glUniform1f(glGetUniformLocation(shaderId, innercCutOffUniform.c_str()), glm::cos(glm::radians(innerCutOffRadius)));
    glUniform1f(glGetUniformLocation(shaderId, outerCutOffUniform.c_str()), glm::cos(glm::radians(outerCutOffRadius)));
    glUniform1f(glGetUniformLocation(shaderId, intensityUninform.c_str()), intensity);
}

PointLight::PointLight(
    glm::vec3 position,
    glm::vec3 lightColor,
    glm::vec3 diffuseColor,
    glm::vec3 ambientColor,
    glm::vec3 specularColor): BaseLight(LightType::Point, position)
{
    this->position = position;
    this->diffuseColor = lightColor * diffuseColor;
    this->ambientColor = lightColor * diffuseColor * ambientColor;
    this->specularColor = specularColor;
}

void PointLight::attachShaderUniforms(
    GLuint shaderId,
    std::string positionUniform,
    std::string diffuseUniform,
    std::string intensityUniform)
{
    glUniform3f(glGetUniformLocation(shaderId, positionUniform.c_str()), position.x, position.y, position.z);
    glUniform3f(glGetUniformLocation(shaderId, diffuseUniform.c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
    glUniform1f(glGetUniformLocation(shaderId, intensityUniform.c_str()), this->intensity);
}

void Lights::Render(GLuint shaderID)
{
    sendDirectionalLightsDataToShader(shaderID);
    sendSpotLightsDataToShader(shaderID);
    sendPointLightsDataToShader(shaderID);
}

void Lights::sendDirectionalLightsDataToShader(GLuint ShaderId)
{
    for (int i = 0; i < directionalLights.size(); i++)
    {
        std::stringstream directionalLightDirection_ss;
        directionalLightDirection_ss << "dirLights[" << i << "].direction";

        std::stringstream color_ss;
        color_ss << "dirLights[" << i << "].color";

        std::stringstream intensity_ss;
        intensity_ss << "dirLights[" << i << "].intensity";

        glUniform1i(glGetUniformLocation(ShaderId, "numberOfDirectionalLights"), directionalLights.size());

        //For each light call attachShaderUniforms
        directionalLights[i].attachShaderUniforms(
            ShaderId,
            directionalLightDirection_ss.str(),
            color_ss.str(),
            intensity_ss.str());
    }
}

void Lights::sendPointLightsDataToShader(GLuint ShaderId)
{
    for (int i = 0; i < pointLights.size(); i++)
    {
        std::stringstream pointLightPosition_ss;
        pointLightPosition_ss << "pointLights[" << i << "].position";

        std::stringstream pointLightDiffuse_ss;
        pointLightDiffuse_ss << "pointLights[" << i << "].diffuse";

        std::stringstream intensity_ss;
        intensity_ss << "pointLights[" << i << "].intensity";

        //For each light call attachShaderUniforms
        pointLights[i].attachShaderUniforms(
            ShaderId,
            pointLightPosition_ss.str(),
            pointLightDiffuse_ss.str(),
            intensity_ss.str());
    }
}

void Lights::sendSpotLightsDataToShader(GLuint ShaderId)
{
    for (int i = 0; i < spotLights.size(); i++)
    {
        std::stringstream spotLightPosition_ss;
        spotLightPosition_ss << "spotLights[" << i << "].position";

        std::stringstream spotLightDirection_ss;
        spotLightDirection_ss << "spotLights[" << i << "].direction";
        
        std::stringstream spotLightColor_ss;
        spotLightColor_ss << "spotLights[" << i << "].color";

        std::stringstream intensity_ss;
        intensity_ss << "spotLights[" << i << "].intensity";

        std::stringstream spotLightInnerCutOff_ss;
        spotLightInnerCutOff_ss << "spotLights[" << i << "].innerCutoff";

        std::stringstream spotLightOuterCutOff_ss;
        spotLightOuterCutOff_ss << "spotLights[" << i << "].outerCutoff";

        glUniform1i(glGetUniformLocation(ShaderId, "numSpotLights"), spotLights.size());

        //For each light call attachShaderUniforms
        spotLights[i].attachShaderUniforms(
            ShaderId,
            spotLightPosition_ss.str(),
            spotLightDirection_ss.str(),
            spotLightColor_ss.str(),
            intensity_ss.str(),
            spotLightInnerCutOff_ss.str(),
            spotLightOuterCutOff_ss.str()
        );
    }
}