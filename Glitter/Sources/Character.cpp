#include <Character/Character.hpp>
#include <filesystem>
#include <Controls/Input.hpp>
#include <EngineState.hpp>
#include <Controls/Controller.hpp>
#include <Controls/PlayerController.hpp>
#include <Modals/CameraType.hpp>

#include "GenericFactory.hpp"
#include <Profiler.hpp>
#include <limits>
namespace fs = std::filesystem;

std::unordered_map<uint32_t, Character*> Character::capsuleCharacterLookup{};

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

    model = new Model(
        filepath,
        EngineState::state->engineInstalledDirectory,
        &skeleton->m_BoneInfoMap,
        &skeleton->m_BoneCounter,
        onModelComponentsLoad);

    skeleton->BuildBoneHierarchy();

    auto playerController = std::make_shared<Controls::PlayerController>(filename);
    EngineState::state->playerControllers.push_back(playerController);
    controller = playerController;

    capsuleCollider = new Physics::Capsule(&getPhysicsSystem(),0.5, 1.0f, true, true);
    capsuleCollider->setOwnerRenderable(this, getInstanceId());
    capsuleCollider->syncTransformation();

    modelRelativePosition = glm::vec3(0.0f);

    camera = new Camera("charactercamera");
    camera->cameraPos = model->GetPosition();
    float pitchAngle = 0.3f;
    glm::quat pitchQuat = glm::angleAxis(pitchAngle, glm::vec3(1, 0, 0));
    glm::quat newRot = pitchQuat * model->GetRot();
    this->camera->cameraFront = glm::rotate(newRot, glm::vec3(0.0f, 0.0f, 1.0f));
    this->camera->cameraUp = glm::rotate(newRot, glm::vec3(0.0f, 1.0f, 0.0f));
    getActiveLevel().cameras.push_back(camera);
    syncCapsuleCharacterLookup();
};

Character::~Character()
{
    unregisterCapsuleCharacterLookup();
    EngineState::state->playerControllers.clear();
    delete model;
    delete animator;
    delete skeleton;
    delete capsuleCollider;
    delete camera;
}

Character* Character::getCharacterByCapsuleId(uint32_t capsuleCharacterId)
{
    const auto found = capsuleCharacterLookup.find(capsuleCharacterId);
    if (found == capsuleCharacterLookup.end())
    {
        return nullptr;
    }

    return found->second;
}

std::vector<Character*> Character::getCollidingCharacters() const
{
    std::vector<Character*> collidingCharacters;
    const auto collidingIds = getCollidingCharacterCapsuleIds();
    collidingCharacters.reserve(collidingIds.size());

    for (const uint32_t capsuleId : collidingIds)
    {
        Character* character = Character::getCharacterByCapsuleId(capsuleId);
        if (character != nullptr && character != this)
        {
            collidingCharacters.push_back(character);
        }
    }

    return collidingCharacters;
}

bool Character::hasCollidingCharacterWithTag(const std::string& tag) const
{
    const auto collidingCharacters = getCollidingCharacters();
    for (const auto* character : collidingCharacters)
    {
        if (character != nullptr && character->hasGameplayTag(tag))
        {
            return true;
        }
    }

    return false;
}

std::vector<Renderable*> Character::GetOverlappingSensors() const
{
    auto* self = const_cast<Character*>(this);
    return getPhysicsSystem().GetOverlappingSensorsFor(self->getInstanceId());
}

void Character::syncGameplayTagSet()
{
    gameplayTagSet.clear();
    gameplayTagSet.insert(gameplayTags.begin(), gameplayTags.end());
}

void Character::syncCapsuleCharacterLookup()
{
    const auto invalidId = std::numeric_limits<uint32_t>::max();
    if (capsuleCollider == nullptr)
    {
        unregisterCapsuleCharacterLookup();
        return;
    }

    const uint32_t newId = capsuleCollider->getCharacterId();
    if (newId == invalidId)
    {
        unregisterCapsuleCharacterLookup();
        return;
    }

    if (registeredCapsuleCharacterId == newId)
    {
        return;
    }

    unregisterCapsuleCharacterLookup();
    capsuleCharacterLookup[newId] = this;
    registeredCapsuleCharacterId = newId;
}

void Character::unregisterCapsuleCharacterLookup()
{
    const auto invalidId = std::numeric_limits<uint32_t>::max();
    if (registeredCapsuleCharacterId == invalidId)
    {
        return;
    }

    const auto found = capsuleCharacterLookup.find(registeredCapsuleCharacterId);
    if (found != capsuleCharacterLookup.end() && found->second == this)
    {
        capsuleCharacterLookup.erase(found);
    }

    registeredCapsuleCharacterId = invalidId;
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
    if (!characterPrefab.stateMachineGuid.empty())
    {
        auto statemachine = std::make_shared<Controls::StateMachine>();
        auto statemachinePath = getEngineRegistryFilesMap()[characterPrefab.stateMachineGuid];
        statemachine->LoadSMfile(statemachinePath);
        character->animStateMachine = statemachine;
    }

    if (character->capsuleCollider != nullptr)
    {
        character->capsuleCollider->setOwnerRenderable(character.get(), character->getInstanceId());
        character->capsuleCollider->setPhysicsLayerName(characterPrefab.capsulePhysicsLayer);
        character->capsuleCollider->setIsSensor(characterPrefab.capsuleIsSensor);
        character->capsuleCollider->syncTransformation();
    }
    character->setGameplayTags(characterPrefab.gameplayTags);

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

    //TURNING OFF updating bone matrix data on CPU vertices.
    // if(animator != nullptr)
    // {
    //     auto transforms = animator->GetFinalBoneMatrices();
    //     for ( int i = 0; i < model->getMeshes()->size(); i++)
    //     {
    //         auto vertices = &model->getMeshes()->at(i).vertices;
    //         for(int j = 0; j < vertices->size(); j++)
    //         {
    //             vertices->at(j).animatedPos = glm::vec4(0.0f);
    //             for (int k = 0; k < 4; k++) {
    //                 int boneID = vertices->at(j).m_BoneIDs[k];
    //                 float weight = vertices->at(j).m_Weights[k];
    //
    //                 if (boneID >= 0) {
    //                     glm::mat4 boneTransform = transforms[boneID];
    //                     vertices->at(j).animatedPos += weight * (boneTransform * glm::vec4(vertices->at(j).Position, 1.0f));
    //                 }
    //             }
    //         }
    //     }
    // }

    //For Debuggging
    // if(model != nullptr)
    // {
    //     model->shader->setInt("displayBoneIndex", getUIState().selectedBoneId);
    // }
}

void Character::drawGeometryOnly(float deltaTime)
{
    uploadBoneMatricesToGPU();

    if(model)
    model->drawGeometryOnly(deltaTime);
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
    ZoneScopedN("CharacterGizmoManipulateDraw");
    ImGuizmo::Manipulate(
glm::value_ptr(viewMatrix),
glm::value_ptr(projMatrix), getUIState().whichTransformActive, ImGuizmo::MODE::WORLD, glm::value_ptr(getModelMatrix()));
}

void Character::draw(float deltaTime, Camera *camera, Lights *lights, CubeMap *cubeMap)
{
    ZoneScopedN("CharacterDraw");
    syncCapsuleCharacterLookup();
    uploadBoneMatricesToGPU();

    if(model)
    {
        if (animator)
        {
            const auto& finalBoneMatrix = animator->GetFinalBoneMatrices();
            model->draw(deltaTime, camera, lights, cubeMap, &finalBoneMatrix);
        }
        else
        {
            model->draw(deltaTime, camera, lights, cubeMap, nullptr);
        }

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
            capsuleCollider->moveBody(
                deltaTime,
                movementOffset,
                rotationOffset,
                isJumping,
                movementSpeed
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

void Character::setFinalBoneMatrix(int boneIndex, glm::mat4 transform) const
{
    for (auto & mesh : model->meshes)
    {
        mesh.mMaterial->GetShader()->setMat4("finalBonesMatrices[" + std::to_string(boneIndex) + "]", transform);
    }
}


void Character::uploadBoneMatricesToGPU() const
{
    if (animator != nullptr)
    {
        const auto finalBoneMatrix = animator->GetFinalBoneMatrices();
    }
    else
    {
        bool isAnimated = false;
    }

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
    syncGameplayTagSet();
    auto model_guid = this->model_guid;
    auto skeleton_guid = this->skeleton_guid;
    auto stateMachine_guid = this->animStateMachine_guid;

    auto filesMap = EngineState::state->engineRegistry->renderableSaveFileMap;

    //load model
    auto model_location = fs::path(filesMap[model_guid]);
    auto model = new Model();

    auto engineFSPath = fs::path(EngineState::state->engineInstalledDirectory);
    auto vertShaderPath = engineFSPath / "Shaders/pbr.vert";
    auto fragShaderPath = engineFSPath / "Shaders/pbr.frag";
    auto material = std::make_shared<Materials::Material>("material", vertShaderPath.u8string().c_str(),fragShaderPath.u8string().c_str());
    //TODO: how to use this material and still be able to assign materialInstances to meshes.
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
    this->capsuleCollider->setOwnerRenderable(this, getInstanceId());
    this->capsuleCollider->syncTransformation();
    syncCapsuleCharacterLookup();

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