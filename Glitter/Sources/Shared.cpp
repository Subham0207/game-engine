#include "Helpers/Shared.hpp"
#include <iostream>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glad/glad.h>
#include <3DModel/Animation/Animation.hpp>
#include <EngineState.hpp>

namespace Shared{
        unsigned int sendTextureToGPU(unsigned char* data, int mWidth, int mheight, int nrComponents){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mheight, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            // std::cout << "Opened and loaded " << path << std::endl;
        }
        else
        {
                std::cerr << "Failed to load texture: "
                << ", Reason: " << stbi_failure_reason() << std::endl;
            stbi_image_free(data);
        }
        return textureID;
    }

    unsigned int TextureFromFile(const char* path, std::string filename, bool save)
    {
        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if(save)
        stbi_write_png(filename.c_str(), width, height, nrComponents, data, 0);
        unsigned int textureID = sendTextureToGPU(data, width, height, nrComponents);
        return textureID;
    }
}

unsigned int Shared::generateMetallicTexture()
{
    unsigned char pixel[4] = {0, 0, 0, 0};
    return Shared::generateTexture(pixel);
}
unsigned int Shared::generateNonMetallicTexture()
{
    unsigned char pixel[4] = {255, 255, 255, 255};
    return Shared::generateTexture(pixel);
}
unsigned int Shared::generateWhiteAOTexture()
{
    unsigned char pixel[4] = {255, 255, 255, 255};
    return Shared::generateTexture(pixel);
}
unsigned int Shared::generateFlatNormalTexture()
{
    unsigned char pixel[4] = {128, 128, 255, 255};
    return Shared::generateTexture(pixel);
}

void Shared::WriteTextFile(const fs::path& p, const std::string& s)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    out.write(s.data(), static_cast<std::streamsize>(s.size()));
}

unsigned int Shared::generateTexture(unsigned char* pixel)
{
unsigned int emptyTexture;
glGenTextures(1, &emptyTexture);
glBindTexture(GL_TEXTURE_2D, emptyTexture);

// Define a single black pixel (RGBA)
// unsigned char blackPixel[4] = {255, 255, 255, 255}; // Black and fully transparent
// Alternatively: {255, 255, 255, 255} for white and opaque

// Allocate the texture with this single pixel
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

// Set texture parameters to avoid sampling artifacts
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

glBindTexture(GL_TEXTURE_2D, 0); // Unbind
return emptyTexture;
}

void Shared::readAnimation(std::string filename)
{
    auto animation = new Animation(filename);
    getUIState().animations.push_back(animation);
    getUIState().animationNames.push_back(animation->animationName);
}

bool Shared::endsWith(const std::string& value, const std::string& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}