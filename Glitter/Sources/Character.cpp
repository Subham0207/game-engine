#include <Character/Character.hpp>
#include <filesystem>
#include <EngineState.hpp>
namespace fs = std::filesystem;

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
    if(animator->m_CurrentAnimation)
    {
        animator->UpdateAnimation(deltatime, m_BoneInfoMap);
    }

    if(animator != nullptr && animator->isAnimationPlaying)
    {
        auto transforms = animator->GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
        {
            setFinalBoneMatrix(i, transforms[i]);
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

void Character::draw()
{
    model->draw();
    bonePositions.clear();
    auto transforms = animator->GetFinalBoneMatrices();
    for (int i = 0; i < m_BoneInfoMap.size(); ++i)
    {
        extractBonePositions(i, transforms[i]);
    }
    renderBones();
}

void Character::bindCubeMapTextures(CubeMap *cubemap)
{
    model->bindCubeMapTextures(cubemap);
}

void Character::useAttachedShader()
{
    model->useAttachedShader();
}

unsigned int Character::getShaderId() const
{
    return model->getShaderId();
}

void Character::setFinalBoneMatrix(int boneIndex, glm::mat4 transform)
{
    model->shader->use();
    model->shader->setMat4("finalBonesMatrices[" + std::to_string(boneIndex) + "]", transform);
}

void Character::extractBonePositions(int boneIndex, glm::mat4 transform)
{
    auto it = m_BoneInfoMap.begin();
    std::advance(it, boneIndex);
    auto boneinfo = it->second;
    glm::vec3 bonePosition = glm::vec3(transform * boneinfo.offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    bonePositions.push_back(bonePosition);
}

void Character::setupBoneBuffersOnGPU()
{
    bonesShader->use();

    glGenVertexArrays(1, &bonesVAO);
    glGenBuffers(1, &bonesVBO);
    glBindVertexArray(bonesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);

    glBufferData(GL_ARRAY_BUFFER, bonePositions.size() * sizeof(glm::vec3), &bonePositions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}
void Character::renderBones()
{
    bonesShader->use();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(bonesVAO);

    // Update vertex buffer with bone positions
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bonePositions.size() * sizeof(glm::vec3), bonePositions.data());

    // Render as points
    glDrawArrays(GL_POINTS, 0, bonePositions.size());
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    model->shader->use();
}