#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

class State{
public:
    State(){}
    static State* state;
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    glm::vec3 rayEnd;

    bool isWorldSpace = true;

    std::string projectRootLocation = fs::current_path().string();

    void printMat4(glm::mat4 mat)
    {
        std::cout << "mat[0]: " << mat[0][0] <<" "<< mat[0][1] <<" "<< mat[0][2] << " " << mat[0][3] << std::endl;
        std::cout << "mat[1]: " << mat[1][0] <<" "<< mat[1][1] <<" "<< mat[1][2] << " " << mat[1][3] << std::endl;
        std::cout << "mat[2]: " << mat[2][0] <<" "<< mat[2][1] <<" "<< mat[2][2] << " " << mat[2][3] << std::endl;
        std::cout << "mat[3]: " << mat[3][0] <<" "<< mat[3][1] <<" "<< mat[3][2] << " " << mat[3][3] << std::endl;
    }
};