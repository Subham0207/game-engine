#include <Character/Character.hpp>
#include <filesystem>
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
    animStateMachine = new Controls::AnimationStateMachine(playerController, animator);

    blendSpace.AddBlendPoint(glm::vec2(0.0f, 0.0f), getUIState().animations[0]);
    blendSpace.AddBlendPoint(glm::vec2(1.0f, 0.0f), getUIState().animations[0]);
    blendSpace.AddBlendPoint(glm::vec2(2.0f, 0.0f), getUIState().animations[0]);

    blendSpace.AddBlendPoint(glm::vec2(1.0f, 1.0f), getUIState().animations[1]);
    blendSpace.AddBlendPoint(glm::vec2(1.0f, 2.0f), getUIState().animations[2]);
    blendSpace.AddBlendPoint(glm::vec2(0.0f, 2.0f), getUIState().animations[4]);
    blendSpace.AddBlendPoint(glm::vec2(2.0f, 2.0f), getUIState().animations[5]);

    blendSpace.AddBlendPoint(glm::vec2(0.0f, 1.0f), getUIState().animations[4]);
    blendSpace.AddBlendPoint(glm::vec2(2.0f, 1.0f), getUIState().animations[5]);

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
        float xfactor = playerController->movementDirection;
        float yfactor = playerController->movementSpeed;
        // float xfactor = getUIState().xblendFactor;
        // float yfactor = getUIState().yblendFactor;
        auto blendSelection = blendSpace.GetBlendSelection(glm::vec2(xfactor, yfactor));
        this->animator->PlayAnimationBlended(blendSelection);
        playerController->update();
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