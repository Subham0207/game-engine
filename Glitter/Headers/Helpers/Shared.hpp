#include <string.h>
#include <iostream>
namespace Shared {
    unsigned int sendTextureToGPU(unsigned char* data, int mWidth, int mheight, int nrComponents);

    unsigned int TextureFromFile(const char* path, std::string filename, bool save = true);

    unsigned int generateTexture(unsigned char* pixel);

    unsigned int generateMetallicTexture();
    unsigned int generateNonMetallicTexture();
    unsigned int generateWhiteAOTexture();

    void readAnimation(std::string filename);
}