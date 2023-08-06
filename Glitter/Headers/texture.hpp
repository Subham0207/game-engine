#pragma once
#include <string>

#include <glad/glad.h>

#include <stb_image.h>

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    Texture(GLenum TextureTarget);

    // Should be called once to load the texture
    bool Load();

    void Load(u32 BufferSize, void* pData);

    void Load(const std::string& Filename);

    void LoadRaw(int Width, int Height, int BPP, unsigned char* pData);

    // Must be called at least once for the specific texture unit
    void Bind(GLenum TextureUnit);

    void GetImageSize(int& ImageWidth, int& ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }

private:
    void LoadInternal(void* image_data);

    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
#ifdef USE_IMAGE_MAGICK
    Magick::Image m_image;
    Magick::Blob m_blob;
#endif
};