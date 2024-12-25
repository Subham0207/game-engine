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

    model->useAttachedShader();
    if(animator != nullptr && animator->isAnimationPlaying)
    {
        auto transforms = animator->GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            model->shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
    }
    else{
        //MAXBONES 100
            for (int i = 0; i < 100; ++i)
            model->shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", 1.0f);
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