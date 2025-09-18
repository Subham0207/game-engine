#pragma once
#include <string>
#include <imgui.h>
#include <vector>

namespace ProjectAsset{
    //Difference between FileExplorer and AssetBrowser is: fileExplorer can open things where as AssetBrowser cannot.
    //So FileExplorer can contain an AssetBrowser
    void RenderFileExplorer(
        std::string& currentPath,
        std::vector<std::string>& fileNames);

    void saveAFile(std::string& currentPath,
                std::vector<std::string>& fileNames,
                bool& showUI);

    void selectOrLoadAFileFromFileExplorer(std::string& currentPath,
                std::vector<std::string>& fileNames,
                bool& showUI);

    bool InputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);

    void createANewProject(
        std::string& currentPath,
        std::vector<std::string>& fileNames,
        bool& showUI);

    void openAProject(
        std::string& currentPath,
        std::vector<std::string>& fileNames,
        bool& showUI
    );
};