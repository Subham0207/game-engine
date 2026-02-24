//
// Created by subha on 24-02-2026.
//

#include "../Headers/RenderPipeline/LightingPass.hpp"

void LightingPass::draw(
    const std::vector<std::shared_ptr<Renderable>>& renderables,
    Camera* activeCamera,
    Lights* lightSystem,
    CubeMap* cubeMap,
    const float deltaTime
)
{
    for(int i=0;i<renderables.size();i++)
    {
        if(renderables.at(i)->ShouldRender())
        {
            renderables.at(i)->useAttachedShader();
            //TODO: to also attach shadowMap of other light sources. And update the shader accordingly
            glActiveTexture(GL_TEXTURE0 + 9);
            glBindTexture(GL_TEXTURE_2D, lightSystem->directionalLights[0].shadowMap);
            renderables[i]->draw(deltaTime, activeCamera, lightSystem, cubeMap);
        }
    }

}
