#include <string>
#include <vector>

namespace ProjectAsset{
    //Difference between FileExplorer and AssetBrowser is: fileExplorer can open things where as AssetBrowser cannot.
    //So FileExplorer can contain an AssetBrowser
    void RenderFileExplorer(
        std::string currentPath,
        std::vector<std::string> fileNames);
};