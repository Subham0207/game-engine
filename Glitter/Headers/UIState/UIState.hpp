#pragma once
#include <vector>

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


        bool showFileDialog = false;
        bool loadModelWindow = false;
        bool showOpenButton = false;

        //FileExplorer
        bool showFileExplorerWindow = false;
        std::vector<std::string> fileNames;
        int selectedFileIndex;
        std::vector<std::string> directoryStack;

        std::string fileExtension="";
        std::string filePath="";

        FileTypeOperation fileTypeOperation;

        Character* character = NULL;
        Shader* shaderOfSelectedModel = NULL;
        int selectedBoneId = 0;

        // Details of ModelType selection window
        std::string modelfileName="";
        std::string albedo="";
        std::string normal="";
        std::string metalness="";
        std::string roughness="";
        std::string ao="";
        std::string saveAsFileName = "";

        std::vector<Animation*> animations;
        std::vector<const char*> animationNames;
        int selectedAnimationIndex = -1;
    };

}