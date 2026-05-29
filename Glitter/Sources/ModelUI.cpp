//
// Created by subha on 01-03-2026.
//

#include "../Headers/UI/ModelUI/ModelUI.hpp"

#include "EngineState.hpp"
#include "imgui.h"
#include "3DModel/model.hpp"

#include <algorithm>

namespace
{
    const char* kBodyTypeLabels[] = {"Rigid body", "Soft body"};
    const char* kMotionTypeLabels[] = {"Static", "Dynamic", "Kinematic"};
    const char* kColliderShapeLabels[] = {"Box", "Sphere", "Capsule", "Custom"};
}

UI::ModelUI::ModelUI()
{
    showUI = false;
    ModelName.setText("Model");
    customColliderAssetPath.clear();
    colliderValidationMessage.clear();
}

UI::ModelUI::~ModelUI()
= default;

void UI::ModelUI::start(const std::shared_ptr<Model>& model)
{
    selectedModel = model;
    materialListComponent.startMaterialsList();
    showUI = true;
}

void UI::ModelUI::draw()
{
    if (!showUI)
        return;

    if (ImGui::Begin(ModelName.value.c_str(), &showUI, ImGuiWindowFlags_AlwaysAutoResize))
    {
        materialListComponent.drawMaterialsList(selectedModel.get());

        if (selectedModel != nullptr)
        {
            bool hasPhysicsBody = selectedModel->getPhysicsBodySettings().has_value();
            if (ImGui::Checkbox("Enable Physics Body", &hasPhysicsBody))
            {
                if (hasPhysicsBody)
                {
                    selectedModel->setPhysicsBodySettings(Physics::PhysicsBodySettings{});
                }
                else
                {
                    selectedModel->setPhysicsBodySettings(std::nullopt);
                    colliderValidationMessage.clear();
                }
            }

            if (hasPhysicsBody)
            {
                auto settings = selectedModel->getPhysicsBodySettings().value_or(Physics::PhysicsBodySettings{});

                int bodyTypeIndex = static_cast<int>(settings.bodyType);
                if (ImGui::Combo("Body Type", &bodyTypeIndex, kBodyTypeLabels, IM_ARRAYSIZE(kBodyTypeLabels)))
                {
                    settings.bodyType = static_cast<Physics::BodyType>(bodyTypeIndex);
                }

                int motionTypeIndex = static_cast<int>(settings.motionType);
                if (ImGui::Combo("Motion Type", &motionTypeIndex, kMotionTypeLabels, IM_ARRAYSIZE(kMotionTypeLabels)))
                {
                    settings.motionType = static_cast<Physics::MotionType>(motionTypeIndex);
                }

                if (settings.bodyType == Physics::BodyType::RigidBody)
                {
                    int colliderShapeIndex = static_cast<int>(settings.rigidBodyData.colliderShape);
                    if (ImGui::Combo("Collider Shape", &colliderShapeIndex, kColliderShapeLabels, IM_ARRAYSIZE(kColliderShapeLabels)))
                    {
                        settings.rigidBodyData.colliderShape = static_cast<Physics::ColliderShape>(colliderShapeIndex);
                    }

                    ImGui::SeparatorText("Transformation Offset");
                    ImGui::DragFloat3("Offset Position", &settings.rigidBodyData.transformationOffset.position.x, 0.05f);
                    ImGui::DragFloat3("Offset Rotation", &settings.rigidBodyData.transformationOffset.rotation.x, 0.5f);
                    ImGui::DragFloat3("Offset Scale", &settings.rigidBodyData.transformationOffset.scale.x, 0.05f, 0.01f, 1000.0f);

                    if (settings.rigidBodyData.colliderShape == Physics::ColliderShape::Custom)
                    {
                        if (settings.motionType != Physics::MotionType::Static)
                        {
                            colliderValidationMessage = "Custom collider supports only Static motion type.";
                        }
                        else
                        {
                            colliderValidationMessage.clear();
                        }

                        char customColliderPathBuffer[512]{};
                        const size_t copyCount = std::min(customColliderAssetPath.size(), sizeof(customColliderPathBuffer) - 1);
                        std::copy(customColliderAssetPath.begin(), customColliderAssetPath.begin() + static_cast<std::string::difference_type>(copyCount), customColliderPathBuffer);
                        if (ImGui::InputText("Custom Collider Mesh", customColliderPathBuffer, sizeof(customColliderPathBuffer)))
                        {
                            customColliderAssetPath = customColliderPathBuffer;
                        }
                        if (ImGui::Button("Cook Custom Collider Geometry"))
                        {
                            if (settings.motionType != Physics::MotionType::Static)
                            {
                                colliderValidationMessage = "Switch motion type to Static before cooking custom geometry.";
                            }
                            else
                            {
                                std::string error;
                                if (!selectedModel->setCustomColliderGeometryFromFile(customColliderAssetPath, &error))
                                {
                                    colliderValidationMessage = "Failed to cook custom collider: " + error;
                                }
                                else
                                {
                                    colliderValidationMessage = "Custom collider geometry cooked successfully.";
                                    settings = selectedModel->getPhysicsBodySettings().value_or(settings);
                                }
                            }
                        }

                        if (!colliderValidationMessage.empty())
                        {
                            ImGui::TextWrapped("%s", colliderValidationMessage.c_str());
                        }
                    }
                }

                selectedModel->setPhysicsBodySettings(settings);
                ImGui::TextWrapped("Physics body settings are serialized, but collider runtime rebuild requires engine restart.");
            }
        }

        if (ImGui::Button("Save"))
        {
            if (selectedModel != nullptr)
            {
                auto path = EngineState::navIntoProjectDir("Assets/");
                selectedModel->save(path);
            }
        }
    }
    ImGui::End();
}
