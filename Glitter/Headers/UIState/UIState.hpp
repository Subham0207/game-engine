#pragma once
#include <vector>
#include <filesystem>
#include <imgui.h>
#include <ImGuizmo.h>
#include <Renderable/renderable.hpp>
#include <Modals/material.hpp>
namespace fs = std::filesystem;

class Model;
class Animation;
class Character;
class Shader;
namespace UI
{
    class CharacterUI;
    class Blendspace2DUI;
};

namespace ProjectAsset {

    enum FileTypeOperation {
        LoadLvlFile, importModelFile, saveModel, loadModel,albedoTexture, normalTexture, metalnessTexture, roughnessTexture, aoTexture, saveLevel, saveLevelAs,
        loadAnimation
    };

    struct UIState{
        std::vector<Renderable*> renderables;
        int selectedRenderableIndex = -1;
        int coordinateSpace = -1;
        bool isFirstFrame = true;

        //ProjectManager
        bool createANewProject = false;
        bool openAProject = false;
        std::string newProjectName;
        std::vector<fs::path> recent_projects;

        bool selectAFile = false;
        bool saveAFile = false;
        bool loadModelWindow = false;
        bool showOpenButton = false;

        //FileExplorer
        std::vector<std::string> fileNames;
        int selectedFileIndex;
        std::vector<std::string> directoryStack;
        std::string currentPath = fs::current_path().string();

        std::string fileExtension="";
        std::string filePath="";

        int materialIndex = -1;
        FileTypeOperation fileTypeOperation;

        Shader* shaderOfSelectedModel = NULL;
        int selectedBoneId = -1;

        // Details of ModelType selection window
        std::string modelfileName="";
        std::vector<std::shared_ptr<Modals::Material>> materials;
        std::string saveAsFileName = "";

        std::vector<Animation*> animations;
        std::vector<std::string> animationNames;
        int selectedAnimationIndex = -1;

        unsigned int metalicTextureID;
        unsigned int nonMetalicTextureID;
        unsigned int whiteAOTextureID;

        bool showDebugBone = false;

        float xblendFactor = 0.0f;
        float yblendFactor = 0.0f;

        ImGuizmo::OPERATION whichTransformActive = ImGuizmo::OPERATION::TRANSLATE;

        //Blendspace
        ImVec2 scrubbedPoint = ImVec2(0,0);

        UI::CharacterUI* characterUIState;
        UI::Blendspace2DUI* blendspace2DUIState;

        UIState();
    };

}