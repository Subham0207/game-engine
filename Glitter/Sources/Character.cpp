#include <Character/Character.hpp>
#include <filesystem>
#include <Controls/Input.hpp>
#include <EngineState.hpp>
#include <Controls/Controller.hpp>
#include <Controls/PlayerController.hpp>
#include <Modals/CameraType.hpp>

#include "GenericFactory.hpp"
namespace fs = std::filesystem;

Character::Character(std::string filepath): Serializable(){
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

    auto playerController = std::make_shared<Controls::PlayerController>(filename);
    EngineState::state->playerControllers.push_back(playerController);
    controller = playerController;

    capsuleCollider = new Physics::Capsule(&getPhysicsSystem(),0.5, 1.0f, true, true);

    modelRelativePosition = glm::vec3(0.0f);

    camera = new Camera("charactercamera");
    camera->cameraPos = model->GetPosition();
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

void Character::loadPrefabIntoActiveLevel(const CharacterPrefabConfig& characterPrefab)
{
    auto character = CharacterFactory::Create(characterPrefab.classId);
    auto filesMap = getEngineRegistryFilesMap();
    if (const auto it = filesMap.find(characterPrefab.modelGuid); it != filesMap.end())
    {
        auto model = new Model();
        auto modelParentPath = fs::path(filesMap[characterPrefab.modelGuid]).parent_path();
        model->load(modelParentPath, characterPrefab.modelGuid);
        character->model = model;
        character->model_guid = characterPrefab.modelGuid;
    }
    if (const auto it = filesMap.find(characterPrefab.skeletonGuid); it != filesMap.end())
    {
        auto skeleton = new Skeleton::Skeleton();
        auto skeletonParentPath = fs::path(filesMap[characterPrefab.skeletonGuid]).parent_path();
        skeleton->load(skeletonParentPath, characterPrefab.skeletonGuid);
        character->skeleton = skeleton;
        character->skeleton_guid = characterPrefab.skeletonGuid;
    }
    if (!characterPrefab.stateMachineClassId.empty())
    {
        auto statemachine = StateMachineFactory::Create(characterPrefab.stateMachineClassId);
    }

    getActiveLevel().renderables.emplace_back(character);
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

void Character::imguizmoManipulate(glm::mat4 viewMatrix, glm::mat4 projMatrix)
{
    ImGuizmo::Manipulate(
glm::value_ptr(viewMatrix),
glm::value_ptr(projMatrix), getUIState().whichTransformActive, ImGuizmo::MODE::WORLD, glm::value_ptr(getModelMatrix()));
}

void Character::draw(float deltaTime, Camera *camera, Lights *lights, CubeMap *cubeMap)
{
    if(animator)
        updateFinalBoneMatrix(deltaTime);

    if(model)
    {
        model->draw(deltaTime, camera, lights, cubeMap);

        auto characterWorldPos = GetPosition();
        auto relativePosition =  glm::vec3(
            characterWorldPos.x + modelRelativePosition.x,
            characterWorldPos.y + modelRelativePosition.y,
            characterWorldPos.z + modelRelativePosition.z
        );

        model->setTransform(relativePosition, GetRot(), GetScale());
    }

    if (capsuleCollider)
        capsuleCollider->tick();

    if(skeleton)
        skeleton->draw(camera, getModelMatrix());

    if(EngineState::state->isPlay)
    {
        if (!started)
        {
            this->onStart();
            started = true;
        }
        else
        {
            this->onTick();
        }

        if(controller)
        {
            if(animStateMachine != nullptr)
            animStateMachine->tick(animator);
        }

        if (capsuleCollider)
        {
            // TODO: Add setIsJumping() and setWalkSpeed() methods in CapsuleCollider. So we can set these values from the derived character class.
            bool isJumping = false;
            bool dodgeStart = false;
            capsuleCollider->moveBody(
                deltaTime,
                movementOffset,
                rotationOffset,
                isJumping,
                dodgeStart ? 8.0f: 4.0f
            );

            setWorldTransform(capsuleCollider->getWorldPosition(), capsuleCollider->getWorldRotation());
        }

    }
    else
    {
        // this logic can just stay in characterBase class.
        started = false;
        if (animator)
        {
            animator->blendSelection = nullptr;
            animator->m_CurrentAnimation = nullptr;
        }
        if(capsuleCollider && capsuleCollider->physics)
        {
            capsuleCollider->setWorldPosition(GetPosition());
            capsuleCollider->setWorldRotation(GetRot());
        }
    }
}

void Character::useAttachedShader()
{
    model->useAttachedShader();
}

void Character::setFinalBoneMatrix(int boneIndex, glm::mat4 transform)
{
    model->shader->setMat4("finalBonesMatrices[" + std::to_string(boneIndex) + "]", transform);
}

void Character::physicsUpdate()
{
}

void Character::syncTransformationToPhysicsEntity()
{
    //The capsule mesh will be attached to the character so when character moves that mesh updates
    //that should be enough to update the physics collider correctly
    // capsuleCollider->syncTransformation();
}


void Character::saveContent(fs::path contentFile, std::ostream& os)
{
    auto loc = EngineState::state->currentActiveProjectDirectory;

    //save model
    this->model->save(contentFile.parent_path());
    this->model_guid = model->GetGuid();

    //save skeleton
    this->skeleton->save(contentFile.parent_path());
    this->skeleton_guid = skeleton->getAssetId();

    Character::saveToFile(contentFile.string(), *this);
}

void Character::loadStateMachine(std::string stateMachine_guid)
{
    auto filesMap = EngineState::state->engineRegistry->renderableSaveFileMap;
    auto stateMachine_Location = fs::path(filesMap[stateMachine_guid]);
    this->animStateMachine = std::make_shared<Controls::StateMachine>();
    this->animStateMachine->load(stateMachine_Location.parent_path(), stateMachine_guid);
    this->animStateMachine_guid = stateMachine_guid;
}

void Character::deleteStateMachine()
{
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

    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertShaderPath = engineFSPath / "Shaders/basic.vert";
    auto fragShaderPath = engineFSPath / "Shaders/pbr.frag";
    model->shader = new Shader(vertShaderPath.u8string().c_str(),fragShaderPath.u8string().c_str());
    model->load(model_location.parent_path(), model_guid);
    this->model = model;

    //create new animator
    this->animator = new Animator();

    //load skeleton
    auto skeleton_Location = fs::path(filesMap[skeleton_guid]);
    this->skeleton = new Skeleton::Skeleton();
    this->skeleton->load(skeleton_Location.parent_path(), skeleton_guid);

    //create new player controller 
    auto playerController = std::make_shared<Controls::PlayerController>(contentName());
    EngineState::state->playerControllers.push_back(playerController);
    controller = playerController;

    //load statemachine
    auto stateMachine_Location = fs::path(filesMap[stateMachine_guid]);
    this->animStateMachine = std::make_shared<Controls::StateMachine>();
    this->animStateMachine->load(stateMachine_Location.parent_path(), stateMachine_guid);


    auto radius = this->capsuleCollider->mRadius;
    auto halfHeight = 2.0f;
    delete this->capsuleCollider;

    //Create new capsule collider
    this->capsuleCollider = new Physics::Capsule(&getPhysicsSystem(), radius, halfHeight, true, true);

    //Create new camera
    camera = new Camera("charactercamera");
    camera->setFOV(70.0f);
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