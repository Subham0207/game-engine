#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>

class Camera
{
friend class InputHandler;
public:
	Camera();
	void updateMVP(unsigned int shader);
	glm::vec3 getPosition();
	glm::vec3 getFront();
	void Camera::FrameModel(const aiAABB& boundingBox);

	glm::mat4 viewMatrix(){
		return view;
	}

	glm::mat4 projectionMatrix(){
		return projection;
	}
	
	glm::vec3 getCameraLookAtDirectionVector(){
		return cameraFront;
	}

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
protected:
private:
	void setupView();
	void setupProjection();
	void lookAt(glm::vec3 whereToLook);
	unsigned int viewLoc;
	unsigned int projectionLoc;
	float fov = 45.0f;

	glm::mat4 view;
	 glm::mat4 projection = glm::mat4(1.0f);
};