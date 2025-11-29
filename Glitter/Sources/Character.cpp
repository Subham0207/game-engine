#include <Character/Character.hpp>
#include <filesystem>
#include <Controls/Input.hpp>
#include <EngineState.hpp>
#include <Controls/PlayerController.hpp>
#include <Modals/CameraType.hpp>
namespace fs = std::filesystem;

Character::Character(std::string filepath){
    filename = fs::path(filepath).filename().string();

    animator = new Animator();
    skeleton = new Skeleton::Skeleton();
    skeleton->setup(filename);

    auto onModelComponentsLoad = [this, filepath](Assimp::Importer* import, const aiScene* scene) {
        if (!scene) {
            std::cerr << "Scene is null!\n";
            return;
        }
        skeleton->ReadHierarchyData(skeleton->m_RootNode, scene->mRootNode);
        //The animation ReadMissingBone and this function seems to do the same thing
        Helpers::resolveBoneHierarchy(scene->mRootNode, -1, skeleton->m_BoneInfoMap, skeleton->m_Bones);
    };

    model = new Model(filepath, &skeleton->m_BoneInfoMap, &skeleton->m_BoneCounter, onModelComponentsLoad);

    skeleton->BuildBoneHierarchy();

    playerController = new Controls::PlayerController(filename);
    EngineState::state->playerControllers.push_back(playerController);
    playerController->register_bindings(getLuaEngine());

    capsuleCollider = new Physics::Capsule(&getPhysicsSystem(),0.5, 1.0f, true, true);

    capsuleColliderPosRelative = glm::vec3(0.0f);

    camera = new Camera("charactercamera");
    camera->cameraPos = model->GetPosition();
    camera->cameraType = CameraType::THIRD_PERSON;
    float pitchAngle = 0.3f;
    glm::quat pitchQuat = glm::angleAxis(pitchAngle, glm::vec3(1, 0, 0));
    glm::quat newRot = pitchQuat * model->GetRot();
    this->camera->cameraFront = glm::rotate(newRot, glm::vec3(0.0f, 0.0f, 1.0f));
    this->camera->cameraUp = glm::rotate(newRot, glm::vec3(0.0f, 1.0f, 0.0f));
    getActiveLevel().cameras.push_back(camera);
};

Character::~Character()
{
    EngineState::state->playerControllers.clear();
    delete playerController;
    delete animStateMachine;
    delete model;
    delete animator;
    delete skeleton;
    delete capsuleCollider;
    delete camera;
}

void Character::saveToFile(std::string filename, Character &character)
{
    fs::path dir = fs::path(filename).parent_path();
    if (dir.empty()) {
        // Set the directory to the current working directory
        dir = fs::current_path();
    }
    if (!fs::exists(dir)) {
        if (!fs::create_directories(dir)) {
            std::cerr << "Failed to create directories: " << dir << std::endl;
            return;
        }
    }
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << character;
    ofs.close();
}

void Character::loadFromFile(const std::string &filename, Character &character)
{
        std::ifstream ifs(filename);
        boost::archive::text_iarchive ia(ifs);
        ia >> character;
}

void Character::updateFinalBoneMatrix(float deltatime)
{
    animator->UpdateAnimation(
        deltatime,
        skeleton->m_BoneInfoMap,
        getModelMatrix(),
        skeleton->bonePositions,
        skeleton->skeletaltreeRoot,
        skeleton->m_Bones);

    if(animator != nullptr)
    {
        auto transforms = animator->GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
        {
            // Apply finalBoneMatrix to GPU
            setFinalBoneMatrix(i, transforms[i]);
        }

        // Apply finalBoneMatrix to CPU mesh as well since we use CPU based selection; TODO: Later add this to a flag so we can switch to other selection methods
        // each vertex has info which bone is influencing it and its position
        //Use mostly the same code in vertex shader to calculate the position of the vertex
        //But if the vertex.pos is what we set ?
        for ( int i = 0; i < model->getMeshes()->size(); i++)
        {
            auto vertices = &model->getMeshes()->at(i).vertices;
            for(int j = 0; j < vertices->size(); j++)
            {
                vertices->at(j).animatedPos = glm::vec4(0.0f);
                for (int k = 0; k < 4; k++) {
                    int boneID = vertices->at(j).m_BoneIDs[k];
                    float weight = vertices->at(j).m_Weights[k];
                    
                    if (boneID >= 0) {
                        glm::mat4 boneTransform = transforms[boneID];
                        vertices->at(j).animatedPos += weight * (boneTransform * glm::vec4(vertices->at(j).Position, 1.0f));
                    }
                }
            }
        }
    }
    else{
        //MAXBONES 100
        for (int i = 0; i < 100; ++i)
        {
            glm::mat4 identityMatrix = glm::mat4(1.0f);
            setFinalBoneMatrix(i, identityMatrix);
        }
    }

    //For Debuggging
    if(model != nullptr)
    {
        model->shader->setInt("displayBoneIndex", getUIState().selectedBoneId);
    }
}

void Character::drawGeometryOnly()
{
    if(model)
    model->drawGeometryOnly();
}

std::vector<ProjectModals::Vertex> Character::GetWorldVertices()
{
    return model->GetWorldVertices();
}
std::vector<unsigned int> Character::GetIndices()
{
    return model->GetIndices();
}
void Character::draw(float deltaTime, Camera *camera, Lights *lights, CubeMap *cubeMap)
{
    if(animator)
    updateFinalBoneMatrix(deltaTime);

    if(model)
    model->draw(deltaTime, camera, lights, cubeMap);

    if(skeleton)
    skeleton->draw(camera, getModelMatrix());

    if(EngineState::state->isPlay)
    {
        //--
        if(this->camera && this->camera->cameraType == CameraType::TOP_DOWN)
        {
            this->camera->cameraPos = (model->GetPosition() - glm::vec3(0,0,cameraDistance)) + glm::vec3(0,cameraHeight,0);
        }
        else 
        {
            glm::vec3 position = model->GetPosition();
            position.y+=5.0f;
            this->camera->tick(position, glm::degrees(glm::eulerAngles(model->GetRot())));
        }
        //--
        
        //-- Turn to Mouse position on XZ plane
        forwardVector = glm::rotate( model->GetRot(), glm::vec3(0.0f, 0.0f, 1.0f));
        rightVector = model->GetRot() * glm::vec3(1.0f, 0.0f, 0.0f);

        if(playerController)
        {
            //Only execute if playerController active otherwise produces nan.
           
            glm::quat desiredRot;
            if(EngineState::state->playerControllers[EngineState::state->activePlayerControllerId] == playerController && this->camera)
            {
                switch (this->camera->cameraType)
                {
                    case CameraType::TOP_DOWN:
                    {
                        playerController->cameraType = CameraType::TOP_DOWN;
                        desiredRot = playerController->faceMouseOnXZ(
                            model->GetPosition(),
                            InputHandler::currentInputHandler->lastX,
                            InputHandler::currentInputHandler->lastY,
                            this->camera->viewMatrix(),
                            this->camera->projectionMatrix()
                        );
                        break;
                    }
                    case CameraType::THIRD_PERSON:
                    {
                        playerController->cameraType = CameraType::THIRD_PERSON;
                        // when third person camera
                        //cameraFront.Y rotation is used when W key is hit to face forward.
                        //LEFT, RIGHT AND Backward will be adjusted accordingly using cameraFront.
                        auto dir = glm::vec3(playerController->inputXWorld, 0.0f, playerController->inputZWorld);
                        if(glm::length(dir) > 0.00001f) // Essentially means W_PRESSED then face camera direction
                        {
                            if(playerController->isAiming)
                            {
                                desiredRot =  glm::angleAxis(this->camera->theta, glm::vec3(0,1,0));
                                this->camera->lastPlayerYaw = this->camera->theta;
                            }
                            else
                            {
                                // calculate desired rotation based on camera view
                                auto cameraFront = this->camera->getFront();
                                auto cameraRight = this->camera->getRight();
                                glm::vec3 forwardXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
                                glm::vec3 rightXZ = glm::normalize(glm::vec3(cameraRight.x, 0.0f, cameraRight.z));
                                glm::vec3 moveDir = forwardXZ * playerController->inputZWorld 
                                - rightXZ   * playerController->inputXWorld;
                                float targetYaw = std::atan2(moveDir.x, moveDir.z);
                                float yaw = smoothAngle(this->camera->lastPlayerYaw, targetYaw, playerController->interpolationSpeed);
                                this->camera->lastPlayerYaw = yaw;
                                desiredRot = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
                            }
                        }
                        else
                        {
                            desiredRot =  glm::angleAxis(this->camera->lastPlayerYaw, glm::vec3(0,1,0));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            else
            desiredRot =  glm::identity<glm::quat>();

            //--
            
            float xfactor = playerController->movementDirection;
            float yfactor = playerController->movementSpeed;
            getUIState().scrubbedPoint.x = xfactor;
            getUIState().scrubbedPoint.y = yfactor;
            
            if(!animStateMachine_guid.empty())
            animStateMachine->tick(playerController, animator);
            
            //apply force to capsule in direction
            if(capsuleCollider && capsuleCollider->physics)
            {
                float x = 0.0f,z = 0.0f;
                switch (this->camera->cameraType)
                {
                    case CameraType::TOP_DOWN:
                    {
                        x = playerController->dodgeStart ? playerController->lookDirection.x :playerController->inputXWorld;
                        z = playerController->dodgeStart ? playerController->lookDirection.z :playerController->inputZWorld;
                        break;
                    }
                    case CameraType::THIRD_PERSON:
                    {
                        auto cameraFront = this->camera->getFront();
                        auto cameraRight = this->camera->getRight();
                        glm::vec3 forwardXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
                        glm::vec3 rightXZ = glm::normalize(glm::vec3(cameraRight.x, 0.0f, cameraRight.z));

                        glm::vec3 moveDir = forwardXZ * playerController->inputZWorld 
                  - rightXZ   * playerController->inputXWorld;
                        x = moveDir.x;
                        z = moveDir.z;
                        break;
                    }                
                    default:
                        break;
                }

                //Right now dodging is thought from top-down camera.
                //Even when idle x,z will have a value based on mouse pos on screen and dodge animation will play moving the character to x,z. Unless mouse is directly on top of the player ( which we have not handled yet)
                //In third person camera when idle x,z won't have value. Lets still move the player by some x,z based on if dodging in player facing direction if idle. In other cases we still dodge to moving direction already.
                capsuleCollider->movebody(
                    x,
                    0.0f,
                    z,
                    deltaTime,
                    model->GetPosition(),
                    desiredRot,
                    playerController->isJumping,
                    playerController->dodgeStart ? 8.0f: 4.0f
                );
                
                playerController->grounded = capsuleCollider ? capsuleCollider->grounded: false;
                playerController->update(forwardVector, rightVector, model->GetRot(), model->getModelMatrix());
                
                auto capsuleWorldPos = capsuleCollider->model->GetPosition();
                auto relativePosition =  glm::vec3(
                    capsuleWorldPos.x + capsuleColliderPosRelative.x,
                    capsuleWorldPos.y + capsuleColliderPosRelative.y,
                    capsuleWorldPos.z + capsuleColliderPosRelative.z
                );
                
                model->setTransformFromPhysics(relativePosition, capsuleCollider->model->GetRot());
            }
        }
    }
    else
    {
        animator->blendSelection = nullptr;
        animator->m_CurrentAnimation = nullptr;
        if(capsuleCollider && capsuleCollider->physics)
        {
            auto characterWorldPos = model->GetPosition();
            auto relativePosition =  glm::vec3(
                characterWorldPos.x - capsuleColliderPosRelative.x,
                characterWorldPos.y - capsuleColliderPosRelative.y,
                characterWorldPos.z - capsuleColliderPosRelative.z
            );
    
            capsuleCollider->model->setTransformFromPhysics(relativePosition, model->GetRot());
            capsuleCollider->position = relativePosition;
            capsuleCollider->rotation = model->GetRot();
        }
    }
}

void Character::useAttachedShader()
{
    model->useAttachedShader();
}

void Character::setFinalBoneMatrix(int boneIndex, glm::mat4 transform)
{
    model->shader->use();
    model->shader->setMat4("finalBonesMatrices[" + std::to_string(boneIndex) + "]", transform);
}

void Character::physicsUpdate()
{
    capsuleCollider->PhysicsUpdate();
}

void Character::syncTransformationToPhysicsEntity()
{
    //The capsule mesh will be attached to the character so when character moves that mesh updates
    //that should be enough to update the physics collider correctly
    capsuleCollider->syncTransformation();
}


void Character::saveContent(fs::path contentFile, std::ostream& os)
{
    auto loc = EngineState::state->currentActiveProjectDirectory;

    //save model
    this->model->save(contentFile.parent_path());
    this->model_guid = model->GetGuid();

    //save skeleton
    this->skeleton->save(contentFile.parent_path());
    this->skeleton_guid = skeleton->getGUID();

    //save statemachine
    this->animStateMachine->save(contentFile.parent_path());
    this->animStateMachine_guid = animStateMachine->getGUID();

    Character::saveToFile(contentFile.string(), *this);
}

void Character::loadStateMachine(std::string stateMachine_guid)
{
    auto filesMap = EngineState::state->engineRegistry->renderableSaveFileMap;
    auto stateMachine_Location = fs::path(filesMap[stateMachine_guid]);
    this->animStateMachine = new Controls::StateMachine();
    this->animStateMachine->load(stateMachine_Location.parent_path(), stateMachine_guid);
    this->animStateMachine_guid = stateMachine_guid;
}

void Character::deleteStateMachine()
{
    this->animStateMachine_guid = "";
    delete this->animStateMachine;
}

void Character::loadContent(fs::path contentFile, std::istream& is)
{
    Character::loadFromFile(contentFile.string(), *this);
    auto model_guid = this->model_guid;
    auto skeleton_guid = this->skeleton_guid;
    auto stateMachine_guid = this->animStateMachine_guid;

    auto filesMap = EngineState::state->engineRegistry->renderableSaveFileMap;

    //load model
    auto model_location = fs::path(filesMap[model_guid]);
    auto model = new Model();
    model->shader = new Shader("./Shaders/basic.vert","./Shaders/pbr.frag");
    model->load(model_location.parent_path(), model_guid);
    this->model = model;

    //create new animator
    this->animator = new Animator();

    //load skeleton
    auto skeleton_Location = fs::path(filesMap[skeleton_guid]);
    this->skeleton = new Skeleton::Skeleton();
    this->skeleton->load(skeleton_Location.parent_path(), skeleton_guid);

    //create new player controller 
    playerController = new Controls::PlayerController(contentName());
    EngineState::state->playerControllers.push_back(playerController);
    playerController->register_bindings(getLuaEngine());

    //load statemachine
    auto stateMachine_Location = fs::path(filesMap[stateMachine_guid]);
    this->animStateMachine = new Controls::StateMachine();
    this->animStateMachine->load(stateMachine_Location.parent_path(), stateMachine_guid);


    auto radius = this->capsuleCollider->radius;
    auto halfHeight = 2.0f;
    delete this->capsuleCollider;

    //Create new capsule collider
    this->capsuleCollider = new Physics::Capsule(&getPhysicsSystem(), radius, halfHeight, true, true);

    //Create new camera
    camera = new Camera("charactercamera");
    camera->setFOV(70.0f);
    camera->cameraType = CameraType::THIRD_PERSON;
    camera->cameraPos = model->GetPosition();
    float pitchAngle = 0.3f;
    glm::quat pitchQuat = glm::angleAxis(pitchAngle, glm::vec3(1, 0, 0));
    glm::quat newRot = pitchQuat * model->GetRot();
    this->camera->cameraFront = glm::rotate(newRot, glm::vec3(0.0f, 0.0f, 1.0f));
    this->camera->cameraUp = glm::rotate(newRot, glm::vec3(0.0f, 1.0f, 0.0f));
    getActiveLevel().cameras.push_back(camera);
}

float Character::smoothAngle(float current, float target, float t)
{
    using glm::pi;
    using glm::two_pi;

    float diff = target - current;

    // Wrap diff into [-pi, pi] to get the shortest path
    while (diff >  glm::pi<float>())  diff -= glm::two_pi<float>();
    while (diff < -glm::pi<float>())  diff += glm::two_pi<float>();

    return current + diff * t;
}