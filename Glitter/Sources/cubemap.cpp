//
// Created by subha on 25-02-2026.
//

#include <Lights/cubemap.hpp>

#include "EngineState.hpp"
#include "stb_image_write.h"

void CubeMap::writeTextureToDisk(int width, int height, unsigned int textureId, bool isMipmapped, bool is2D, std::string name)
{
    // 1. Handle 2D Textures (BRDF LUT)
    if (is2D) {
        float* data = new float[width * height * 3];
        glBindTexture(GL_TEXTURE_2D, textureId);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);

        auto path = EngineState::navIntoProjectDir("Assets/IBL/brdf_lut.hdr"s);;
        stbi_write_hdr(path.string().c_str(), width, height, 3, data);

        delete[] data;
        return;
    }

    // 2. Handle Cubemaps (Irradiance & Prefiltered)
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    // Prefiltered maps usually have 5 mips (0 to 4)
    int maxMipLevels = isMipmapped ? 5 : 1;

    for (int mip = 0; mip < maxMipLevels; ++mip) {
        int mipWidth  = width  >> mip;
        int mipHeight = height >> mip;
        float* data = new float[mipWidth * mipHeight * 3];

        for (unsigned int face = 0; face < 6; face++) {
            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                          mip,
                          GL_RGB,
                          GL_FLOAT,
                          data);

            auto path = "Assets"s + "/IBL/"s + name + "_"
                                   + std::to_string(mip) + "_face_"s + std::to_string(face) + ".hdr"s;
            auto filename = EngineState::navIntoProjectDir(path);

            stbi_write_hdr(filename.string().c_str(), mipWidth, mipHeight, 3, data);
        }
        delete[] data;
    }
}