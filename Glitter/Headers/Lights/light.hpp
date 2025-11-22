#pragma once
#include "glm/glm.hpp"
#include "glad/glad.h"
#include <sstream>
#include <string>
#include <vector>
#include <Modals/LightType.hpp>
class Model;

class BaseLight
{
public:
	BaseLight(LightType lightType, glm::vec3 pos);
	Model* lightModel;
};

class DirectionalLight: public BaseLight {
public:
	glm::vec3 direction;

	glm::vec3 diffuseColor;
	glm::vec3 ambientColor;
	glm::vec3 specularColor;

	DirectionalLight(
		glm::vec3 position, // Note we take position to update the light 3d model but for directional light position does not matter.
		glm::vec3 direction,
		glm::vec3 lightColor,
		glm::vec3 diffuseColor  = glm::vec3(0.5f),
		glm::vec3 ambientColor  = glm::vec3(0.2f),
		glm::vec3 specularColor = glm::vec3(1.0f)): BaseLight(LightType::Directional, position)
	{
		this->direction = direction;
		this->diffuseColor = lightColor * diffuseColor;
		this->ambientColor = lightColor * diffuseColor * ambientColor;
		this->specularColor = specularColor;
	}

	void attachShaderUniforms(
		GLuint shaderId,
		std::string directionUniform,
		std::string ambientUniform,
		std::string diffuseUniform,
		std::string specularUniform)
	{
		glUniform3f(glGetUniformLocation(shaderId, directionUniform.c_str()), direction.x, direction.y, direction.z);
		glUniform3f(glGetUniformLocation(shaderId, ambientUniform.c_str()), ambientColor.x, ambientColor.y, ambientColor.z);
		glUniform3f(glGetUniformLocation(shaderId, diffuseUniform.c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(glGetUniformLocation(shaderId, specularUniform.c_str()), specularColor.x, specularColor.y, specularColor.z);
	}
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

	SpotLight(
		glm::vec3 position,
		glm::vec3 lightColor,
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f),
		float innerCutOffRadius = glm::cos(glm::radians(10.5f)),
		float outerCutOffRadius = glm::cos(glm::radians(17.5f)),
		glm::vec3 diffuseColor = glm::vec3(0.5f),
		glm::vec3 ambientColor = glm::vec3(0.2f),
		glm::vec3 specularColor = glm::vec3(1.0f)): BaseLight(LightType::Spot, position)
	{
		this->position = position;
		this->direction = direction;
		this->innerCutOffRadius = innerCutOffRadius;
		this->outerCutOffRadius = outerCutOffRadius;
		this->diffuseColor = lightColor * diffuseColor;
		this->ambientColor = lightColor * diffuseColor * ambientColor;
		this->specularColor = specularColor;
	}

	float constantTerm = 1.0f;
	float linearTerm = 0.09f;
	float quadraticTerm = 0.032f;

	void attachShaderUniforms(
		GLuint shaderId,
		std::string positionUniform,
		std::string directionUniform,
		std::string innercCutOffUniform,
		std::string outerCutOffUniform,
		std::string ambientUniform,
		std::string diffuseUniform,
		std::string specularUniform,
		std::string constantTermUniform,
		std::string linearTermUniform,
		std::string quadraticUniform)
	{
		glUniform3f(glGetUniformLocation(shaderId, positionUniform.c_str()), position.x, position.y, position.z);
		glUniform3f(glGetUniformLocation(shaderId, directionUniform.c_str()), direction.x, direction.y, direction.z);
		glUniform1f(glGetUniformLocation(shaderId, innercCutOffUniform.c_str()), innerCutOffRadius);
		glUniform1f(glGetUniformLocation(shaderId, outerCutOffUniform.c_str()), outerCutOffRadius);
		glUniform3f(glGetUniformLocation(shaderId, ambientUniform.c_str()), ambientColor.x, ambientColor.y, ambientColor.z);
		glUniform3f(glGetUniformLocation(shaderId, diffuseUniform.c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(glGetUniformLocation(shaderId, specularUniform.c_str()), specularColor.x, specularColor.y, specularColor.z);
		glUniform1f(glGetUniformLocation(shaderId, constantTermUniform.c_str()), constantTerm);
		glUniform1f(glGetUniformLocation(shaderId, linearTermUniform.c_str()), linearTerm);
		glUniform1f(glGetUniformLocation(shaderId, quadraticUniform.c_str()), quadraticTerm);
	}
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

	PointLight(
		glm::vec3 position,
		glm::vec3 lightColor,
		glm::vec3 diffuseColor = glm::vec3(0.5f),
		glm::vec3 ambientColor = glm::vec3(0.2f),
		glm::vec3 specularColor = glm::vec3(1.0f)): BaseLight(LightType::Point, position)
	{
		this->position = position;
		this->diffuseColor = lightColor * diffuseColor;
		this->ambientColor = lightColor * diffuseColor * ambientColor;
		this->specularColor = specularColor;
	}

	void attachShaderUniforms(
		GLuint shaderId,
		std::string positionUniform,
		std::string diffuseUniform)
	{
		glUniform3f(glGetUniformLocation(shaderId, positionUniform.c_str()), position.x, position.y, position.z);
		glUniform3f(glGetUniformLocation(shaderId, diffuseUniform.c_str()), diffuseColor.x, diffuseColor.y, diffuseColor.z);
	}
};

class Lights {
public:
	std::vector<DirectionalLight> directionalLights;
	std::vector<SpotLight> spotLights;
	std::vector<PointLight> pointLights;
	void Render(GLuint shaderID)
	{
		sendDirectionalLightsDataToShader(shaderID);
		sendSpotLightsDataToShader(shaderID);
		sendPointLightsDataToShader(shaderID);
	}

private:
	void sendDirectionalLightsDataToShader(GLuint ShaderId)
	{
		for (int i = 0; i < directionalLights.size(); i++)
		{
			std::stringstream directionalLightDirection_ss;
			directionalLightDirection_ss << "dirLights[" << i << "].direction";

			std::stringstream directionalLightAmbient_ss;
			directionalLightAmbient_ss << "dirLights[" << i << "].ambient";

			std::stringstream directionalLightDiffuse_ss;
			directionalLightDiffuse_ss << "dirLights[" << i << "].diffuse";


			std::stringstream directionalLightSpecular_ss;
			directionalLightSpecular_ss << "dirLights[" << i << "].specular";

			glUniform1i(glGetUniformLocation(ShaderId, "numberOfDirectionalLights"), directionalLights.size());

			//For each light call attachShaderUniforms
			directionalLights[i].attachShaderUniforms(
				ShaderId,
				directionalLightDirection_ss.str(),
				directionalLightAmbient_ss.str(),
				directionalLightDiffuse_ss.str(),
				directionalLightSpecular_ss.str());
		}
	}

	void sendPointLightsDataToShader(GLuint ShaderId)
	{
		for (int i = 0; i < pointLights.size(); i++)
		{
			std::stringstream pointLightPosition_ss;
			pointLightPosition_ss << "pointLights[" << i << "].position";

			std::stringstream pointLightDiffuse_ss;
			pointLightDiffuse_ss << "pointLights[" << i << "].diffuse";

			//For each light call attachShaderUniforms
			pointLights[i].attachShaderUniforms(
				ShaderId,
				pointLightPosition_ss.str(),
				pointLightDiffuse_ss.str());
		}
	}

	void sendSpotLightsDataToShader(GLuint ShaderId)
	{
		for (int i = 0; i < spotLights.size(); i++)
		{
			std::stringstream spotLightPosition_ss;
			spotLightPosition_ss << "spotLights[" << i << "].position";

			std::stringstream spotLightDirection_ss;
			spotLightDirection_ss << "spotLights[" << i << "].direction";

			std::stringstream spotLightInnerCutOff_ss;
			spotLightInnerCutOff_ss << "spotLights[" << i << "].cutoff";

			std::stringstream spotLightOuterCutOff_ss;
			spotLightOuterCutOff_ss << "spotLights[" << i << "].outerCutOff";

			std::stringstream spotLightAmbient_ss;
			spotLightAmbient_ss << "spotLights[" << i << "].ambient";

			std::stringstream spotLightDiffuse_ss;
			spotLightDiffuse_ss << "spotLights[" << i << "].diffuse";


			std::stringstream spotLightSpecular_ss;
			spotLightSpecular_ss << "spotLights[" << i << "].specular";

			std::stringstream spotLightConstantTerm_ss;
			spotLightConstantTerm_ss << "spotLights[" << i << "].constantTerm";

			std::stringstream spotLightLinearTerm_ss;
			spotLightLinearTerm_ss << "spotLights[" << i << "].linearTerm";

			std::stringstream spotLightQuadraticTerm_ss;
			spotLightLinearTerm_ss << "spotLights[" << i << "].quadraticTerm";

			glUniform1i(glGetUniformLocation(ShaderId, "numberOfPointLights"), pointLights.size());

			//For each light call attachShaderUniforms
			spotLights[i].attachShaderUniforms(
				ShaderId,
				spotLightPosition_ss.str(),
				spotLightDirection_ss.str(),
				spotLightInnerCutOff_ss.str(),
				spotLightOuterCutOff_ss.str(),
				spotLightAmbient_ss.str(),
				spotLightDiffuse_ss.str(),
				spotLightSpecular_ss.str(),
				spotLightConstantTerm_ss.str(),
				spotLightLinearTerm_ss.str(),
				spotLightQuadraticTerm_ss.str());
		}
	}
};
