//
// Created by subha on 21-12-2025.
//

#include <cstdlib>

#include <Editor.hpp>

int main(int argc, char * argv[])
{
    bool isDevMode = false;

    // Check if we have at least 2 arguments (exe name + flag + value)
    if (argc >= 3) {
        std::string_view flag = argv[1];
        std::string_view value = argv[2];

        if (flag == "--devMode" && value == "true") {
            isDevMode = true;
        }
    }
    auto editor = new Editor();
    editor->openEditor();

    return EXIT_SUCCESS;
}