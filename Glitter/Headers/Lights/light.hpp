#pragma once
#include "glm/glm.hpp"
#include "glad/glad.h"
#include <sstream>
#include <string>
#include <vector>
#include <Modals/LightType.hpp>
#include <GLFW/glfw3.h>
class Model;
class Shader;
class Camera;
class CubeMap;
class Lights;

class BaseLight
{
public:
	BaseLight(LightType lightType, glm::vec3 pos);
	std::shared_ptr<Model> lightModel;
};

class DirectionalLight: public BaseLight {
public:
	glm::vec3 direction;

	glm::vec3 diffuseColor;
	glm::vec3 ambientColor;
	glm::vec3 specularColor;
	float intensity;

	unsigned int shadowMapFBO;
	unsigned int shadowMap;
	float nearPlane;
	float farPlane;
	float extent;

	Shader* shadowMapShader;
	unsigned int shadowMapWidth,shadowMapHeight;
	glm::mat4 dirLightVP;

	DirectionalLight(
		glm::vec3 position, // Note we take position to update the light 3d model but for directional light position does not matter.
		glm::vec3 direction,
		glm::vec3 lightColor,
		glm::vec3 diffuseColor  = glm::vec3(0.5f),
		glm::vec3 ambientColor  = glm::vec3(0.2f),
		glm::vec3 specularColor = glm::vec3(1.0f));

	void attachShaderUniforms(
		GLuint shaderId,
		std::string directionUniform,
		std::string colorUniform,
		std::string intensityUniform);

	void evaluateShadowMap(GLFWwindow* window, float deltaTime, Camera* activeCamera, Lights *lights, CubeMap *cubeMap);
private:
	void setupShadowObjects();
};

class SpotLight: public BaseLight {
public:
	glm::vec3 direction;
	glm::vec3 position;

	float innerCutOffRadius;
	float outerCutOffRadius;

	glm::vec3 diffuseColor;
	glm::vec3 ambientColor;
	glm::vec3 specularColor;

	GLuint spotDepthFBO = 0;
	GLuint spotDepthMap = 0;
	int spotShadowWidth = 1024;
	int spotShadowHeight = 1024;

	float spotNearPlane = 0.1f;
	float spotFarPlane  = 50.0f;  // adjust to your scene
	glm::mat4 spotLightSpaceMatrix;
	Shader* spotShadowShader;

	SpotLight(
		glm::vec3 position,
		glm::vec3 lightColor,
		glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f),
		float innerCutOffRadius = 10.5f,
		float outerCutOffRadius = 17.5f,
		glm::vec3 diffuseColor = glm::vec3(0.5f),
		glm::vec3 ambientColor = glm::vec3(0.2f),
		glm::vec3 specularColor = glm::vec3(1.0f));

	float constantTerm = 1.0f;
	float linearTerm = 0.09f;
	float quadraticTerm = 0.032f;

	float intensity = 5000.0f;

	void attachShaderUniforms(
		GLuint shaderId,
		std::string positionUniform,
		std::string directionUniform,
		std::string colorUniform,
		std::string intensityUninform,
		std::string innercCutOffUniform,
		std::string outerCutOffUniform
	);

	void setupShadowObjects();
	void evaluateShadowMap(GLFWwindow* window, float deltaTime);
};

class PointLight: public BaseLight {
public:
	glm::vec3 position;

	glm::vec3 diffuseColor;
	glm::vec3 ambientColor;
	glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);

	float constantTerm = 1.0f;
	float linearTerm = 0.09f;
	float quadraticTerm = 0.032f;
	float intensity = 100.0f;

	unsigned int depthMapFBO;
	unsigned int depthCubemap;
	unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
	float nearPlane, farPlane;
	std::vector<glm::mat4> shadowTransforms;
	Shader* shadowMapShader;

	PointLight(
		glm::vec3 position,
		glm::vec3 lightColor,
		glm::vec3 diffuseColor = glm::vec3(0.5f),
		glm::vec3 ambientColor = glm::vec3(0.2f),
		glm::vec3 specularColor = glm::vec3(1.0f));

	void attachShaderUniforms(
		GLuint shaderId,
		std::string positionUniform,
		std::string diffuseUniform,
		std::string intensityUniform);
	
	void evaluateShadowMap(GLFWwindow* window, float deltaTime);
private:
	void setupShadowObjects();
};

class Lights {
public:
	std::vector<DirectionalLight> directionalLights;
	std::vector<SpotLight> spotLights;
	std::vector<PointLight> pointLights;
	void Render(GLuint shaderID);

private:
	void sendDirectionalLightsDataToShader(GLuint ShaderId);

	void sendPointLightsDataToShader(GLuint ShaderId);

	void sendSpotLightsDataToShader(GLuint ShaderId);
};
