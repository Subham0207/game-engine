//
// Created by subha on 24-02-2026.
//

#include "../Headers/RenderPipeline/LightingPass.hpp"
#include <3DModel/model.hpp>
#include "Character/Character.hpp"

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
            //TODO: to also attach shadowMap of other light sources. And update the shader accordingly
            glActiveTexture(GL_TEXTURE0 + 9);
            glBindTexture(GL_TEXTURE_2D, lightSystem->directionalLights[0].shadowMap);
            if (auto model = std::dynamic_pointer_cast<Model>(renderables.at(i)))
                model->draw(deltaTime, activeCamera, lightSystem, cubeMap, nullptr);
            if (auto character = std::dynamic_pointer_cast<Character>(renderables.at(i)))
                character->draw(deltaTime, activeCamera, lightSystem, cubeMap);
            if (auto CapsuleColliderModel = std::dynamic_pointer_cast<Character>(renderables.at(i)))
                CapsuleColliderModel->draw(deltaTime, activeCamera, lightSystem, cubeMap);
        }
    }

}
