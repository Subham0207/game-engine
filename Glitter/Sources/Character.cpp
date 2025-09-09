#include <Character/Character.hpp>
#include <filesystem>
#include <Controls/Input.hpp>
#include <EngineState.hpp>
namespace fs = std::filesystem;

Character::Character(std::string filepath){
    animator = new Animator();
    skeleton = new Skeleton::Skeleton();
    model = new Model(filepath, &skeleton->m_BoneInfoMap, &skeleton->m_BoneCounter);
    skeleton->setup(animator, this->model->getModelMatrix());

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
    skeleton->ReadHierarchyData(skeleton->m_RootNode, scene->mRootNode);

    //The animation ReadMissingBone and this function seems to do the same thing
    Helpers::resolveBoneHierarchy(scene->mRootNode, -1, skeleton->m_BoneInfoMap, skeleton->m_Bones);

    skeleton->BuildBoneHierarchy();

    playerController = new Controls::PlayerController();
    State::state->playerControllers.push_back(playerController);

    animStateMachine = new Controls::StateMachine();
    auto locomotionState = new Controls::State("Locomotion");
    auto jumpState = new Controls::State("Jump");
    auto dodgeRollState = new Controls::State("DodgeRoll");

    locomotionState->toStateWhenCondition->push_back(
        Controls::ToStateWhenCondition(
        jumpState,
        [this]() { return playerController && !playerController->grounded; })
    );
    // locomotionState->toStates->push_back(dodgeRollState);

    jumpState->toStateWhenCondition->push_back(
        Controls::ToStateWhenCondition(
        locomotionState,
        [this]() { return playerController && playerController->grounded; })
    );
    // dodgeRollState->toStates->push_back(locomotionState);

    animStateMachine->setActiveState(locomotionState);

    locomotionState->blendspace = new BlendSpace2D();

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 0.0f), getUIState().animations[0]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 0.0f), getUIState().animations[4]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 0.0f), getUIState().animations[5]);

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 1.0f), getUIState().animations[1]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 1.0f), getUIState().animations[1]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 1.0f), getUIState().animations[1]);

    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, 2.0f), getUIState().animations[2]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, 2.0f), getUIState().animations[2]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, 2.0f), getUIState().animations[2]);

    locomotionState->blendspace->AddBlendPoint(glm::vec2(-1.0f, -1.0f), getUIState().animations[6]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(0.0f, -1.0f), getUIState().animations[6]);
    locomotionState->blendspace->AddBlendPoint(glm::vec2(1.0f, -1.0f), getUIState().animations[6]);

    jumpState->animation = getUIState().animations[7];

    capsuleCollider = new Physics::Capsule(&getPhysicsSystem(), true, true);

    capsuleColliderPosRelative = glm::vec3(0.0f);

    camera = new Camera();
    camera->cameraPos = model->GetPosition();
    float pitchAngle = 0.3f;
    glm::quat pitchQuat = glm::angleAxis(pitchAngle, glm::vec3(1, 0, 0));
    glm::quat newRot = pitchQuat * model->GetRot();
    this->camera->cameraFront = glm::rotate(newRot, glm::vec3(0.0f, 0.0f, 1.0f));
    this->camera->cameraUp = glm::rotate(newRot, glm::vec3(0.0f, 1.0f, 0.0f));
    getActiveLevel().cameras.push_back(camera);
};

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
    character.name = filename;
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << character;
    ofs.close();
}

void Character::loadFromFile(const std::string &filename, Character &character)
{
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

void Character::draw(float deltaTime, Camera* camera, Lights* lights, CubeMap* cubeMap)
{
    updateFinalBoneMatrix(deltaTime);
    model->draw(deltaTime, camera, lights, cubeMap);

    skeleton->draw(camera, getModelMatrix());



    if(State::state->isPlay)
    {
        //--
        State::state->activeCameraIndex = getActiveLevel().cameras.size() - 1;

        this->camera->cameraPos = (model->GetPosition() - glm::vec3(0,0,cameraDistance)) + glm::vec3(0,cameraHeight,0);
        //--
        
        //-- Turn to Mouse position on XZ plane
        forwardVector = glm::rotate( model->GetRot(), glm::vec3(0.0f, 0.0f, 1.0f));
        rightVector = model->GetRot() * glm::vec3(1.0f, 0.0f, 0.0f);
        auto desiredRot = playerController->faceMouseOnXZ(
            model->GetPosition(),
            InputHandler::currentInputHandler->lastX,
            InputHandler::currentInputHandler->lastY,
            this->camera->viewMatrix(),
            this->camera->projectionMatrix()
        );


        //--
        
        float xfactor = playerController->movementDirection;
        float yfactor = playerController->movementSpeed;
        // float xfactor = getUIState().xblendFactor;
        // float yfactor = getUIState().yblendFactor;
        getUIState().scrubbedPoint.x = xfactor;
        getUIState().scrubbedPoint.y = yfactor;

        animStateMachine->tick(playerController, animator);

        //apply force to capsule in direction
        capsuleCollider->movebody(
            playerController->inputXWorld,
            0.0f,
            playerController->inputZWorld,
            deltaTime,
            model->GetPosition(),
            desiredRot,
            playerController->isJumping
        );

        playerController->grounded = capsuleCollider->grounded;
        playerController->update(forwardVector, rightVector, model->GetRot(), model->getModelMatrix());

        auto capsuleWorldPos = capsuleCollider->model->GetPosition();
        auto relativePosition = glm::vec3(
        capsuleWorldPos.x + capsuleColliderPosRelative.x,
        capsuleWorldPos.y + capsuleColliderPosRelative.y,
        capsuleWorldPos.z + capsuleColliderPosRelative.z
        );
        model->setTransformFromPhysics(relativePosition, capsuleCollider->model->GetRot());
    }
    else
    {
        auto characterWorldPos = model->GetPosition();
        auto relativePosition = glm::vec3(
            characterWorldPos.x - capsuleColliderPosRelative.x,
            characterWorldPos.y - capsuleColliderPosRelative.y,
            characterWorldPos.z - capsuleColliderPosRelative.z
        );

        capsuleCollider->model->setTransformFromPhysics(relativePosition, model->GetRot());
        capsuleCollider->position = relativePosition;
        capsuleCollider->rotation = model->GetRot();
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
