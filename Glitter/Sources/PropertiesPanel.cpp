#include <UI/PropertiesPanel.hpp>
#include <imgui.h>
#include <Lights/light.hpp>
#include <3DModel/Model.hpp>

namespace UI
{
    PropertiesPanel::PropertiesPanel()
    {
        resetAll();
    }

    void PropertiesPanel::resetAll()
    {
        spotlight = nullptr;
        directionalLight = nullptr;
        pointLight = nullptr;
    }

    void PropertiesPanel::setPointLight(PointLight* pointLight)
    {
        pointLight = pointLight;
    }

    void PropertiesPanel::setDirectionalLight(DirectionalLight* dl)
    {
        directionalLight = dl;
    }
    void PropertiesPanel::setSpotLight(SpotLight* sl)
    {
        spotlight = sl;
    }

    void PropertiesPanel::draw()
    {
        ImGui::Text("---Properties panel---");
        if(spotlight)
        {
            ImGui::Text("----------");
            ImGui::Text("Spot Light %s", spotlight->lightModel->getName().c_str());

            ImGui::Text("Intensity");
            ImGui::SameLine();
            ImGui::DragFloat("##spotlight-intensity", &spotlight->intensity, 0.005f);

            ImGui::Text("InnerRadius");
            ImGui::SameLine();
            ImGui::DragFloat("##innerradius", &spotlight->innerCutOffRadius, 0.005f);

            ImGui::Text("OuterRadius");
            ImGui::SameLine();
            ImGui::DragFloat("##outerradius", &spotlight->outerCutOffRadius, 0.005f);

            ImGui::Text("Color");
            ImGui::DragFloat("R##spotlight-color", &spotlight->diffuseColor.r, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("G##spotlight-color", &spotlight->diffuseColor.g, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("B##color", &spotlight->diffuseColor.b, 0.005f);

            ImGui::Text("Direction");
            ImGui::DragFloat("X##spotlight-direction", &spotlight->direction.x, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("Y##spotlight-direction", &spotlight->direction.y, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("Z##spotlight-direction", &spotlight->direction.z, 0.005f);
        }
        if(directionalLight)
        {
            ImGui::Text("----------");

            ImGui::Text("Directional Light %s", directionalLight->lightModel->getName().c_str());

            ImGui::Text("Intensity");
            ImGui::SameLine();
            ImGui::DragFloat("##directionallight-intensity", &directionalLight->intensity, 0.005f);

            ImGui::Text("Color");
            ImGui::DragFloat("R##directionallight-color", &directionalLight->diffuseColor.r, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("G##directionallight-color", &directionalLight->diffuseColor.g, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("B##directionallight-color", &directionalLight->diffuseColor.b, 0.005f);
            
            ImGui::Text("Direction");
            ImGui::DragFloat("X##directionallight-direction", &directionalLight->direction.x, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("Y##directionallight-direction", &directionalLight->direction.y, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("Z##directionallight-direction", &directionalLight->direction.z, 0.005f);
        }
        if(pointLight)
        {
            ImGui::Text("----------");

            ImGui::Text("Point Light %s", pointLight->lightModel->getName().c_str());

            ImGui::Text("Intensity");
            ImGui::SameLine();
            ImGui::DragFloat("##pointlight-intensity", &pointLight->intensity, 0.005f);

            ImGui::Text("Color");
            ImGui::DragFloat("R##pointlight-color", &pointLight->diffuseColor.r, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("G##pointlight-color", &pointLight->diffuseColor.g, 0.005f);
            ImGui::SameLine();
            ImGui::DragFloat("B##pointlight-color", &pointLight->diffuseColor.b, 0.005f);
        }
        ImGui::Text("---Properties panel---");
    }
}