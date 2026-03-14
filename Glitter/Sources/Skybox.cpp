//
// Created by subha on 12-03-2026.
//

#include "../Headers/Lights/Skybox.hpp"

#include <filesystem>
#include <glad/glad.h>

#include "Lights/cubemap.hpp"

Lighting::Skybox::Skybox(fs::path basePath, GLFWwindow* window)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //CubeMap -- Blocking 0th textureId for environment map. Models will start using from 1+ index.
    auto cubeMapPath = basePath / "EngineAssets/kloofendal_48d_partly_cloudy_puresky_8k.hdr";

    auto cubeMapVertShader = basePath / "Shaders/cubemap.vert";
    auto cubeMapEquiShader = basePath / "Shaders/equirectanglular_to_cubemap.frag";

    auto cubeMapIrrShader = basePath / "Shaders/irradiance_convolution.frag";
    auto cubeMapPreFilterShader = basePath / "Shaders/prefilter.frag";

    auto cubeMapBRDFVertShader = basePath / "Shaders/brdf.vert";
    auto cubeMapBRDFFragShader = basePath / "Shaders/brdf.frag";

    auto cubeMapBackgroundVertShader = basePath / "Shaders/background.vert";
    auto cubeMapBackgroundFragShader = basePath / "Shaders/background.frag";

    cubeMap = new CubeMap(cubeMapPath.string());
    auto equirectangularToCubemapShader = new Shader(cubeMapVertShader.u8string().c_str(),cubeMapEquiShader.u8string().c_str());
    auto irradianceShader = new Shader(cubeMapVertShader.u8string().c_str(),cubeMapIrrShader.u8string().c_str());
    auto prefilterShader = new Shader(cubeMapVertShader.u8string().c_str(),cubeMapPreFilterShader.u8string().c_str());
    auto brdfShader = new Shader(cubeMapBRDFVertShader.u8string().c_str(),cubeMapBRDFFragShader.u8string().c_str());
    backgroundShader = new Shader(cubeMapBackgroundVertShader.u8string().c_str(),cubeMapBackgroundFragShader.u8string().c_str());

    cubeMap->setup(window,
    *equirectangularToCubemapShader, *irradianceShader, *prefilterShader, *brdfShader);

    //For RGBA to work Enable Alpha channel and Blend;
    //NOTE: Its very important to enable alpha and blend after cubemap generation else brdfLUT will come out black
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

CubeMap* Lighting::Skybox::getCubeMap() const
{
    return cubeMap;
}

void Lighting::Skybox::Draw(glm::mat4& view, glm::mat4& proj) const
{
    cubeMap->Draw(view, proj, backgroundShader);
}
