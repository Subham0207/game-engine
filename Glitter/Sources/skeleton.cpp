#include <3DModel/Skeleton/skeleton.hpp>
#include <EngineState.hpp>
#include <Sprites/text.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>

glm::mat4 Skeleton::Skeleton::worldTransform(int boneIndex, glm::mat4 modelMatrix)
{
    auto it = m_BoneInfoMap.begin();
    std::advance(it, boneIndex);
    auto boneinfo = it->second;
    return it->second.transform * modelMatrix;
}

bool Skeleton::Skeleton::isClose(glm::vec3 parentEndpoint, glm::vec3 childPosition, float tolerance)
{
    return glm::all(glm::epsilonEqual(parentEndpoint, childPosition, tolerance));;
}
void Skeleton::Skeleton::setupBoneBuffersOnGPU()
{
    glGenVertexArrays(1, &bonesVAO);
    glGenBuffers(1, &bonesVBO);
    glGenBuffers(1, &bonesColorVBO);

    glBindVertexArray(bonesVAO);

    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);
    glBufferData(GL_ARRAY_BUFFER, bonePositions.size() * sizeof(glm::vec3), bonePositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, bonesColorVBO);
    glBufferData(GL_ARRAY_BUFFER, boneColors.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Skeleton::Skeleton::draw(Camera* camera, glm::mat4 &modelMatrix)
{
    bonesShader->use();
    updateModelAndViewPosMatrix(camera, modelMatrix);

    boneColors = std::vector<glm::vec3>(bonePositions.size(), glm::vec3(1.0f, 0.0f, 0.0f));

    auto selectedBoneIndex = getUIState().selectedBoneId;
    if (selectedBoneIndex >= 0 && selectedBoneIndex < boneColors.size()) {
        boneColors[selectedBoneIndex] = glm::vec3(0.0f, 1.0f, 0.0f); // Selected bone is green
    }

    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(bonesVAO);

    // Update vertex buffer with bone positions
    glBindBuffer(GL_ARRAY_BUFFER, bonesVBO);
    glBufferData(GL_ARRAY_BUFFER, bonePositions.size() * sizeof(glm::vec3), bonePositions.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Upload bone colors
    glBindBuffer(GL_ARRAY_BUFFER, bonesColorVBO);
    glBufferData(GL_ARRAY_BUFFER, boneColors.size() * sizeof(glm::vec3), boneColors.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    // Render as points
    glDrawArrays(GL_LINES, 0, bonePositions.size());
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}
void Skeleton::Skeleton::setup(std::string filename)
{
    this->filename = filename;
    bonesShader = new Shader("./Shaders/boneShader.vert", "./Shaders/boneShader.frag");
    bonesShader->use();
    setupBoneBuffersOnGPU();
}

void Skeleton::Skeleton::updateModelAndViewPosMatrix(Camera *camera, glm::mat4 &modelMatrix)
{
    glm::mat4 projection = camera->projectionMatrix();
    glm::mat4 view = camera->viewMatrix();
    bonesShader->setMat4("projection", projection);
    bonesShader->setMat4("view", view);
    bonesShader->setMat4("model", modelMatrix);
}

void Skeleton::Skeleton::saveContent(fs::path contentFile, std::ostream& os)
{
    fs::path dir = fs::path(contentFile.string()).parent_path();
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
    std::ofstream ofs(contentFile.string());
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
    ofs.close();
}

void Skeleton::Skeleton::loadContent(fs::path contentFile, std::istream& is)
{
        std::ifstream ifs(contentFile.string());    
        boost::archive::text_iarchive ia(ifs);
        ia >> *this;

        this->setup(this->filename);
        // this->BuildBoneHierarchy();
}