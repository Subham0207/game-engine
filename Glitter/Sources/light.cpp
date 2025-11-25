#include <Lights/light.hpp>
#include <3DModel/model.hpp>
#include <EngineState.hpp>
#include <filesystem>
#include <Modals/3DModelType.hpp>
#include <Helpers/shader.hpp>
#include <EngineState.hpp>
#include <Lights/cubemap.hpp>

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

    setupShadowObjects();
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

void DirectionalLight::evaluateShadowMap(GLFWwindow* window, float deltaTime, Camera* activeCamera, Lights *lights, CubeMap *cubeMap)
{
    glCullFace(GL_FRONT);
    auto lvlrenderables = getActiveLevel().renderables;

	glm::mat4 orthgonalProjection = glm::ortho(-extent, extent, -extent, extent, nearPlane, farPlane);
    auto lightPos = 20.0f * lightModel->GetPosition();
    direction = lightPos - glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightProjection = orthgonalProjection * lightView;

    glEnable(GL_DEPTH_TEST);

    // Preparations for the Shadow Map
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw scene for shadow map
    shadowMapShader->use();
    glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));

    for(int i=0;i<lvlrenderables->size();i++)
    {
        if(lvlrenderables->at(i)->ShouldRender())
        {
            glm::mat4 model = (*lvlrenderables)[i]->getModelMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            (*lvlrenderables)[i]->drawGeometryOnly();
        }
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Shadow FBO incomplete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
    glCullFace(GL_BACK);
}

void DirectionalLight::setupShadowObjects()
{
    shadowMapShader = new Shader("./Shaders/shadowMap.vert", "./Shaders/shadowMap.frag");

	glGenFramebuffers(1, &shadowMapFBO);

	// Texture for Shadow Map FBO
	shadowMapWidth = 2048, shadowMapHeight = 2048;
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Prevents darkness outside the frustrum
	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	// Needed since we don't touch the color buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// Matrices needed for the light's perspective
    nearPlane = 0.1;
	farPlane = 300.0f;
    extent = 200.0f;
	glm::mat4 orthgonalProjection = glm::ortho(-extent, extent, -extent, extent, nearPlane, farPlane);
    auto lightPos = 20.0f * lightModel->GetPosition();
    direction = lightPos - glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightProjection = orthgonalProjection * lightView;

    shadowMapShader->use();
	glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));

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