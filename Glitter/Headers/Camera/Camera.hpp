#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "assimp/scene.h"

#include <serializeAClass.hpp>
#include <Modals/CameraType.hpp>

class Camera
{
friend class InputHandler;
public:
	Camera();
	Camera(std::string name): cameraName(name){}
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

	void render();
	void processDefaultCamera(InputHandler *currentInputHandler);
	void processThirdPersonCamera(InputHandler *currentInputHandler);

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	std::string cameraName;

	//First Person camera
	float yaw = -90.0f;
	float pitch = 0.0f;

	//Third person camera parameters
	float maxDistance = 20.0f;
	float cameraDistance = 16.0f;
	float cameraHeight = 7.0f;
	float angleAroundPlayer;
	CameraType cameraType = CameraType::TOP_DOWN;
	glm::vec3 playerRot;
	glm::vec3 playerPos;
	void calculateAngleAroundPlayer(float offset);
	float calculateHorizontalDistance();
    float calculateVerticalDistance();
    glm::vec3 calculateCameraPosition();

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