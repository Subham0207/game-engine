#include<Editor.hpp>
#include <string>
#include <iostream>

#ifndef GLITTER_ENGINE
#error GLITTER_ENGINE is not defined for this file/target
#endif

int main(int argc, char * argv[])
{
    try
    {
        bool isDevMode = false;
        if (argc >= 3) {
            std::string_view flag = argv[1];
            std::string_view value = argv[2];

            if (flag == "--devMode" && value == "true") {
                isDevMode = true;
            }
        }
        auto editor = new Editor();
        std::string engine_root = GLITTER_ENGINE;
        std::string project_root = CMAKE_ROOT_DIR;
        editor->openEditor(engine_root, project_root, isDevMode);
    }
    catch(std::exception &error)
     {
         std::cout << "ERROR OCCURED: " << error.what() << std::endl;
     }

    return 0;
}