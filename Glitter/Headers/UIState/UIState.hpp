#pragma once
#include <vector>
#include <filesystem>
#include <Modals/material.hpp>
namespace fs = std::filesystem;

class Model;
class Animation;
class Character;
class Shader;

namespace ProjectAsset {

    enum FileTypeOperation {
        LoadLvlFile, importModelFile, saveModel, loadModel,albedoTexture, normalTexture, metalnessTexture, roughnessTexture, aoTexture, saveLevel, saveLevelAs,
        loadAnimation
    };

    struct UIState{
        std::vector<Model*> models;
        int selectedModelIndex = -1;
        int coordinateSpace = -1;
        bool isFirstFrame = true;


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

        Character* character = NULL;
        Shader* shaderOfSelectedModel = NULL;
        int selectedBoneId = 0;

        // Details of ModelType selection window
        std::string modelfileName="";
        std::vector<Modals::Material*> materials;
        std::string saveAsFileName = "";

        std::vector<Animation*> animations;
        std::vector<std::string> animationNames;
        int selectedAnimationIndex = -1;
    };

}