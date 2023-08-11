#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
friend class InputHandler;
public:
	Camera();
	void updateMVP(unsigned int shader);
	glm::vec3 getPosition();
	glm::vec3 getFront();
protected:
private:
	void setupView();
	void setupProjection();
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	unsigned int viewLoc;
	unsigned int projectionLoc;
	float fov = 45.0f;
};

