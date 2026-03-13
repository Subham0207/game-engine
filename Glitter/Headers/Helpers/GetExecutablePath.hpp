//
// Created by subha on 13-03-2026.
//

#ifndef GLITTER_GETEXECUTABLEPATH_HPP
#define GLITTER_GETEXECUTABLEPATH_HPP

#pragma once
#include <filesystem>
namespace fs = std::filesystem;

// We are using a helper function to resolve the directory of the executable.
// It is better than fs::current_path() -- since fs::current_path() is not where your .exe lives. It is where the user was standing when they clicked the button.
// So in terminal we can be in another directory and launch an exe which is another directory.
class GetExecutablePath
{
public:
    fs::path static getExecutableDir();
};


#endif //GLITTER_GETEXECUTABLEPATH_HPP