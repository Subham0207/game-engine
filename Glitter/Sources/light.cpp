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
    auto loc = std::filesystem::path(EngineState::state->engineInstalledDirectory);

    switch (lightType)
    {
        case LightType::Directional :
        {
            lightModel = std::make_shared<Model>((loc / "EngineAssets" / "cube.fbx").string());
            lightModel->setDirName("directionalLight");
            break;
        }
        case LightType::Point :
        {
            
            lightModel = std::make_shared<Model>((loc / "EngineAssets" / "cube.fbx").string());
            lightModel->setDirName("pointLight");
            break;
        }
        case LightType::Spot :
        {
            
            lightModel = std::make_shared<Model>((loc / "EngineAssets" / "cube.fbx").string());
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

void DirectionalLight::evaluateShadowMap(GLFWwindow* window, float deltaTime, unsigned int FBO)
{
    glCullFace(GL_FRONT);
    auto lvlrenderables = getActiveLevel().renderables;

	glm::mat4 orthgonalProjection = glm::ortho(-extent, extent, -extent, extent, nearPlane, farPlane);
    auto lightPos = 20.0f * lightModel->GetPosition();
    direction = lightPos - glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    dirLightVP = orthgonalProjection * lightView;

    glEnable(GL_DEPTH_TEST);

    // Preparations for the Shadow Map
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw scene for shadow map
    shadowMapShader->use();
    glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "dirLightVP"), 1, GL_FALSE, glm::value_ptr(dirLightVP));

    for(int i=0;i<lvlrenderables.size();i++)
    {
        if(lvlrenderables.at(i)->ShouldRender())
        {
            glm::mat4 model = (lvlrenderables)[i]->getModelMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            // Character bone matrix update
            if (auto character = std::dynamic_pointer_cast<Character>(lvlrenderables[i]))
            {
                glUniform1i(glGetUniformLocation(shadowMapShader->ID, "isAnimated"), 1);

                auto transforms = character->animator->GetFinalBoneMatrices();
                for (int j = 0; j < transforms.size(); j++) {
                    std::string name = "finalBonesMatrices[" + std::to_string(j) + "]";
                    shadowMapShader->setMat4(name, transforms[j]);
                }
            } else {
                glUniform1i(glGetUniformLocation(shadowMapShader->ID, "isAnimated"), 0);
            }
            //---------------------------
            (lvlrenderables)[i]->drawGeometryOnly(deltaTime);
        }
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Shadow FBO incomplete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
    glCullFace(GL_BACK);
}

void DirectionalLight::setupShadowObjects()
{
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertpath = engineFSPath / "Shaders/shadowMap.vert";
    auto fragPath = engineFSPath / "Shaders/shadowMap.frag";
    shadowMapShader = new Shader(vertpath.u8string().c_str(), fragPath.u8string().c_str());

	glGenFramebuffers(1, &shadowMapFBO);

	// Texture for Shadow Map FBO
	shadowMapWidth = 4096, shadowMapHeight = 4096;
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
	dirLightVP = orthgonalProjection * lightView;

    shadowMapShader->use();
	glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "dirLightVP"), 1, GL_FALSE, glm::value_ptr(dirLightVP));

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

    SpotLight::setupShadowObjects();
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

    glActiveTexture(GL_TEXTURE0 + 10);
    glBindTexture(GL_TEXTURE_2D, spotDepthMap);
    glUniform1i(glGetUniformLocation(shaderId, "spotShadowMap"), 10);

    // Spot light light-space matrix
    glUniformMatrix4fv(glGetUniformLocation(shaderId, "spotLightSpaceMatrix"),
    1, GL_FALSE, glm::value_ptr(spotLightSpaceMatrix));
}

void SpotLight::setupShadowObjects()
{
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertPath = engineFSPath  / "Shaders/spotlight/depth.vert";
    auto fragPath = engineFSPath / "Shaders/spotlight/depth.frag";
    spotShadowShader = new Shader(
        vertPath.u8string().c_str(),
        fragPath.u8string().c_str()
    );

    glGenFramebuffers(1, &spotDepthFBO);
    glGenTextures(1, &spotDepthMap);

    glBindTexture(GL_TEXTURE_2D, spotDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 spotShadowWidth, spotShadowHeight, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, spotDepthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, spotDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Spotlight shadow framebuffer not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SpotLight::evaluateShadowMap(GLFWwindow *window, float deltaTime, unsigned int FBO)
{
    glCullFace(GL_FRONT);
    auto lvlrenderables = getActiveLevel().renderables;

    // 1. Build light-space matrix (perspective)
    float fov = glm::radians(outerCutOffRadius * 2.0f); // or just some cone angle
    float aspect = 1.0f;

    glm::mat4 lightProj = glm::perspective(fov, aspect, spotNearPlane, spotFarPlane);

    glm::vec3 lightPos = position;         // make sure this matches your spotLights[0].position
    glm::vec3 lightDir = glm::normalize(direction);

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    // if spotlight is nearly parallel to up, you may want a different up, but this is fine for now
    if(lightDir.y == -up.y)
    {
        lightDir.x += 0.0001;
    }

    glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, up);
    spotLightSpaceMatrix = lightProj * lightView;

    glEnable(GL_DEPTH_TEST);
    // 2. Render depth
    glViewport(0, 0, spotShadowWidth, spotShadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, spotDepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    spotShadowShader->use();
    glUniformMatrix4fv(glGetUniformLocation(spotShadowShader->ID, "lightSpaceMatrix"),
                       1, GL_FALSE, glm::value_ptr(spotLightSpaceMatrix));

    for (int i = 0; i < lvlrenderables.size(); ++i)
    {
        if (lvlrenderables.at(i)->ShouldRender())
        {
            glm::mat4 model = (lvlrenderables)[i]->getModelMatrix();
            glUniformMatrix4fv(glGetUniformLocation(spotShadowShader->ID, "model"),
                               1, GL_FALSE, glm::value_ptr(model));
            if (auto character = std::dynamic_pointer_cast<Character>(lvlrenderables[i]))
            {
                glUniform1i(glGetUniformLocation(spotShadowShader->ID, "isAnimated"), 1);

                auto transforms = character->animator->GetFinalBoneMatrices();
                for (int j = 0; j < transforms.size(); j++) {
                    std::string name = "finalBonesMatrices[" + std::to_string(j) + "]";
                    spotShadowShader->setMat4(name, transforms[j]);
                }
            } else {
                glUniform1i(glGetUniformLocation(spotShadowShader->ID, "isAnimated"), 0);
            }
            (lvlrenderables)[i]->drawGeometryOnly(deltaTime);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
    glCullFace(GL_BACK);
}

PointLight::PointLight(
    glm::vec3 position,
    glm::vec3 lightColor,
    glm::vec3 diffuseColor,
    glm::vec3 ambientColor,
    glm::vec3 specularColor) : BaseLight(LightType::Point, position)
{
    this->position = position;
    this->diffuseColor = lightColor * diffuseColor;
    this->ambientColor = lightColor * diffuseColor * ambientColor;
    this->specularColor = specularColor;

    setupShadowObjects();
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

void PointLight::evaluateShadowMap(GLFWwindow* window, float deltaTime, unsigned int FBO)
{
    glCullFace(GL_FRONT);
    auto lvlrenderables = getActiveLevel().renderables;

    glEnable(GL_DEPTH_TEST);

    // 1) Bind shadow FBO and set viewport
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // 2) Use shadow shader
    shadowMapShader->use();

    // 3) Recompute shadow matrices based on current light position
    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);
    glm::vec3 position = lightModel->GetPosition();

    glm::mat4 shadowMatrices[6];
    shadowMatrices[0] = shadowProj * glm::lookAt(position, position + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0));
    shadowMatrices[1] = shadowProj * glm::lookAt(position, position + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0));
    shadowMatrices[2] = shadowProj * glm::lookAt(position, position + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0));
    shadowMatrices[3] = shadowProj * glm::lookAt(position, position + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0));
    shadowMatrices[4] = shadowProj * glm::lookAt(position, position + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0));
    shadowMatrices[5] = shadowProj * glm::lookAt(position, position + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0));

    // 4) Upload shadowMatrices[6]
    for (int i = 0; i < 6; ++i)
    {
        std::string name = "shadowMatrices[" + std::to_string(i) + "]";
        glUniformMatrix4fv(
            glGetUniformLocation(shadowMapShader->ID, name.c_str()),
            1, GL_FALSE, glm::value_ptr(shadowMatrices[i])
        );
    }

    // 5) Upload lightPos and far_plane (names must match GLSL!)
    glUniform3fv(glGetUniformLocation(shadowMapShader->ID, "lightPos"), 1, glm::value_ptr(position));
    glUniform1f(glGetUniformLocation(shadowMapShader->ID, "far_plane"), farPlane);

    // 6) Draw scene with model matrices
    for (int i = 0; i < lvlrenderables.size(); ++i)
    {
        if (lvlrenderables.at(i)->ShouldRender())
        {
            glm::mat4 model = (lvlrenderables)[i]->getModelMatrix();
            glUniformMatrix4fv(glGetUniformLocation(shadowMapShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            if (auto character = std::dynamic_pointer_cast<Character>(lvlrenderables[i]))
            {
                glUniform1i(glGetUniformLocation(shadowMapShader->ID, "isAnimated"), 1);

                auto transforms = character->animator->GetFinalBoneMatrices();
                for (int j = 0; j < transforms.size(); j++) {
                    std::string name = "finalBonesMatrices[" + std::to_string(j) + "]";
                    shadowMapShader->setMat4(name, transforms[j]);
                }
            } else {
                glUniform1i(glGetUniformLocation(shadowMapShader->ID, "isAnimated"), 0);
            }
            (lvlrenderables)[i]->drawGeometryOnly(deltaTime);
        }
    }

    // 7) Restore default FBO and viewport
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
    glCullFace(GL_BACK);
}
void PointLight::setupShadowObjects()
{
    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertPath  = engineFSPath / "Shaders/pointlight/depthshader.vert";
    auto fragPath = engineFSPath / "Shaders/pointlight/depthshader.frag";
    auto geoPath = engineFSPath / "Shaders/pointlight/depthshader.geo";
    shadowMapShader = new Shader(
        vertPath.u8string().c_str(),
        fragPath.u8string().c_str(),
        geoPath.u8string().c_str()
        );

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthCubemap);

    SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        
    }
                    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Point light shadow framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
    nearPlane = 1.0f;
    farPlane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);
    auto position = lightModel->GetPosition();

    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));

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

        std::string cubeName = "pointLights[" + std::to_string(i) + "].depthMap";
        std::string farName  = "pointLights[" + std::to_string(i) + "].farPlane";

        //For each light call attachShaderUniforms
        pointLights[i].attachShaderUniforms(
            ShaderId,
            pointLightPosition_ss.str(),
            pointLightDiffuse_ss.str(),
            intensity_ss.str());
        
        glActiveTexture(GL_TEXTURE0 + 11 + i);             // 11,12,13...
        glBindTexture(GL_TEXTURE_CUBE_MAP, pointLights[i].depthCubemap);
        glUniform1i(glGetUniformLocation(ShaderId, cubeName.c_str()), 11 + i);

        glUniform1f(glGetUniformLocation(ShaderId, farName.c_str()),
        pointLights[i].farPlane);
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