#include <UI/PropertiesPanel.hpp>
#include <imgui.h>
#include <Lights/light.hpp>
#include <3DModel/Model.hpp>

namespace UI
{
    PropertiesPanel::PropertiesPanel()
    {
        spotlight = nullptr;
        directionalLight = nullptr;
        pointLight = nullptr;
    }

    void PropertiesPanel::draw()
    {
        ImGui::Text("---Properties panel---");
        if(spotlight)
        {
            ImGui::Text("Spot Light %s", spotlight->lightModel->getName().c_str());

            ImGui::Text("Intensity");
            ImGui::SameLine();
            ImGui::DragFloat("##intensity", &spotlight->intensity, 0.005f);

            ImGui::Text("InnerRadius");
            ImGui::SameLine();
            ImGui::DragFloat("##innerradius", &spotlight->innerCutOffRadius, 0.005f);

            ImGui::Text("OuterRadius");
            ImGui::SameLine();
            ImGui::DragFloat("##outerradius", &spotlight->outerCutOffRadius, 0.005f);

            ImGui::Text("Color");
            ImGui::DragFloat("R##color", &spotlight->diffuseColor.r, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("G##color", &spotlight->diffuseColor.g, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("B##color", &spotlight->diffuseColor.b, 0.005f);

            ImGui::Text("Direction");
            ImGui::DragFloat("X##direction", &spotlight->direction.x, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("Y##direction", &spotlight->direction.y, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("Z##direction", &spotlight->direction.z, 0.005f);
        }
        ImGui::Text("---Properties panel---");
    }
}