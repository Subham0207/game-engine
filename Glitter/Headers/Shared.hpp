#include <string.h>
#include <iostream>
namespace Shared {
    unsigned int sendTextureToGPU(unsigned char* data, int mWidth, int mheight, int nrComponents);

    unsigned int TextureFromFile(const char* path, std::string filename, bool save = true);
}