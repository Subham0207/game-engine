#include "shared.hpp"
#include <iostream>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glad/glad.h>

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
