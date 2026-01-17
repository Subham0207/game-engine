#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "assimp/scene.h"
#include <serializeAClass.hpp>
#include "Event/Event.hpp"

class Camera
{
friend class InputHandler;
public:
virtual ~Camera() = default;
Camera();
	Camera(std::string name);
	void updateMVP(unsigned int shader);
	glm::vec3 getPosition();
	glm::vec3 getFront();
	glm::vec3 getRight();
	void FrameModel(const aiAABB& boundingBox);
	void setFOV(float fov){this->fov = fov;}

	glm::mat4 viewMatrix(){
		return view;
	}

	glm::mat4 projectionMatrix(){
		return projection;
	}
	
	glm::vec3 getCameraLookAtDirectionVector(){
		return cameraFront;
	}

	virtual void onMouseMove(const MouseMoveEvent& e);

	void tick(glm::vec3 playerPos, glm::vec3 playerRot);

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	std::string cameraName;

	void lookAt(glm::vec3 whereToLook);

protected:
private:
	void setupView();
	void setupProjection();
	unsigned int viewLoc;
	unsigned int projectionLoc;
	float fov = 45.0f;

	glm::mat4 view;
	glm::mat4 projection = glm::mat4(1.0f);

	friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
		ar & viewLoc;
		ar & projectionLoc;
		ar & fov;
		ar & view;
		ar & projection;
		ar & cameraPos;
		ar & cameraFront;
		ar & cameraUp;
    }
};