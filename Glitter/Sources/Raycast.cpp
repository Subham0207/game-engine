//
// Created by subha on 12-03-2026.
//

#include "../Headers/Debug/Raycast.hpp"

#include "imgui.h"
#include "ImGuizmo.h"
#include "Camera/Camera.hpp"
#include "Controls/Input.hpp"
#include "Helpers/raypicking.hpp"
#include "Helpers/shader.hpp"
#include "UI/outliner.hpp"

namespace Debug
{
    Raycast::Raycast(fs::path basePath)
    {
        auto rayVertPath = basePath / "Shaders/rayCast.vert";
        auto rayFragPath = basePath / "Shaders/rayCast.frag";
        rayCastshader =  new Shader(
            rayVertPath.u8string().c_str(),
            rayFragPath.u8string().c_str());
    }

    void Raycast::HandleSelection(Outliner* outliner, Camera* activeCamera, const Level* level, InputHandler* input)
    {
        ImGuizmo::BeginFrame();
        // Set the window and matrix for ImGuizmo
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

        const auto& renderables = level->renderables;

        auto getSelectedIndex = outliner->GetSelectedIndex();
        rayCastshader->use();
        activeCamera->updateMVP(rayCastshader->ID);

        if (!input || !input->m_Camera)
            return;

        auto view = input->m_Camera->viewMatrix();
        auto proj = input->m_Camera->projectionMatrix();
        auto getSelectedIndexFromMouseCurrentFrame = handlePicking(
            input->lastX,
            input->lastY,
            renderables,
            view,
            proj,
            rayCastshader->ID,
            rayOrigin,
            rayDir,
            input->m_Camera->getCameraLookAtDirectionVector()
        );
        if(getSelectedIndexFromMouseCurrentFrame > -2)
            outliner->setSelectedIndex(getSelectedIndexFromMouseCurrentFrame);


        if(getSelectedIndex > -1)
            renderables[getSelectedIndex]->imguizmoManipulate(activeCamera->viewMatrix(), activeCamera->projectionMatrix());

    }
}
