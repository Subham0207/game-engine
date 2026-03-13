#pragma once
#include <string>
#include <imgui.h>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;

namespace ProjectAsset{
    //Difference between FileExplorer and AssetBrowser is: fileExplorer can open things where as AssetBrowser cannot.
    //So FileExplorer can contain an AssetBrowser
    void RenderFileExplorer(
        std::string& currentPath,
        std::vector<std::string>& fileNames,
        std::string& rootPath,
        int& selectedFileIndex,
        std::string& selectedFilePath);

    void saveAFile(std::string& currentPath,
                std::vector<std::string>& fileNames,
                bool& showUI,
                std::string& rootPath,
                int& selectedFileIndex,
                std::string& filePath);

    void selectOrLoadAFileFromFileExplorer(std::string& currentPath,
                std::vector<std::string>& fileNames,
                bool& showUI,
                std::string& rootPath,
                int& selectedFileIndex,
                std::string& filePath);

    bool InputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);

    void createANewProject(
        std::string& currentPath,
        std::vector<std::string>& fileNames,
        bool& showUI,
        std::string& newProjectName,
        std::string& rootPath,
        int& selectedFileIndex,
        std::string& filePath,
        std::vector<fs::path>& recent_projects
        );

    void openAProject(
        std::string& currentPath,
        std::vector<std::string>& fileNames,
        bool& showUI,
        std::string& rootPath,
        int& selectedFileIndex,
        std::string& filePath
    );
};