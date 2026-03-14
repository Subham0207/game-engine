#pragma once
#include <vector>
#include <filesystem>
#include <string>
namespace fs = std::filesystem;

namespace ProjectManagerUI{
    class ProjectManager
    {
        public:
            ProjectManager(std::string currentPath, std::string rootPath);
            void draw();
            void addToRecentProjects(fs::path line);
        private:
            bool createANewProject;
            std::vector<fs::path> recent_projects;
            std::string currentActiveProjectDirectory;
            std::string mCurrentPath;
            std::vector<std::string> fileNames;
            bool openAProject;
            std::string newProjectName;

            std::string mRootPath;
            int mSelectedFileIndex;
            std::string mSelectedFilePath;
    };
}
