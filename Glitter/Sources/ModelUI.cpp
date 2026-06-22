//
// Created by subha on 01-03-2026.
//

#include "../Headers/UI/ModelUI/ModelUI.hpp"

#include "EngineState.hpp"
#include "GenericFactory.hpp"
#include "imgui.h"
#include "3DModel/model.hpp"
#include "Physics/PhysicsLayerRegistry.hpp"
#include "UI/Shared/ComboUI.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
    const char* kBodyTypeLabels[] = {"Rigid body", "Soft body"};
    const char* kMotionTypeLabels[] = {"Static", "Dynamic", "Kinematic"};
    const char* kColliderShapeLabels[] = {"Box", "Sphere", "Capsule", "Custom", "Convex Hull"};

    std::vector<std::string> parseTagsCsv(const std::string& csv)
    {
        std::vector<std::string> tags;
        std::stringstream stream(csv);
        std::string token;
        while (std::getline(stream, token, ','))
        {
            token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), token.end());
            if (!token.empty())
            {
                tags.push_back(token);
            }
        }
        return tags;
    }

    std::string toTagsCsv(const std::vector<std::string>& tags)
    {
        std::string csv;
        for (size_t i = 0; i < tags.size(); ++i)
        {
            csv += tags[i];
            if (i + 1 < tags.size())
            {
                csv += ", ";
            }
        }
        return csv;
    }
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
    registeredModelClassNames.clear();
    selectedModelClassIndex = 0;
    if (selectedModel != nullptr)
    {
        if (selectedModel->classId.empty())
        {
            selectedModel->classId = "None";
        }

        int index = 0;
        for (const auto& [className, creator] : ModelFactory::GetTable())
        {
            if (className == selectedModel->classId)
            {
                selectedModelClassIndex = index + 1;
            }
            registeredModelClassNames.emplace_back(className);
            index += 1;
        }

        if (selectedModelClassIndex == 0 && selectedModel->classId != "None")
        {
            selectedModel->classId = "None";
        }
    }
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
            if (UI::Shared::comboUI("Choose Model Class", selectedModelClassIndex, registeredModelClassNames))
            {
                if (selectedModelClassIndex <= 0)
                {
                    selectedModel->classId = "None";
                }
                else
                {
                    const int selectedClassIndex = selectedModelClassIndex - 1;
                    if (selectedClassIndex >= 0 && selectedClassIndex < static_cast<int>(registeredModelClassNames.size()))
                    {
                        selectedModel->classId = registeredModelClassNames[static_cast<size_t>(selectedClassIndex)];
                    }
                    else
                    {
                        selectedModel->classId = "None";
                    }
                }
            }

            char tagsBuffer[512]{};
            const std::string tagsCsv = toTagsCsv(selectedModel->getGameplayTags());
            const size_t tagsCopyCount = std::min(tagsCsv.size(), sizeof(tagsBuffer) - 1);
            std::copy(tagsCsv.begin(), tagsCsv.begin() + static_cast<std::string::difference_type>(tagsCopyCount), tagsBuffer);
            if (ImGui::InputText("Gameplay Tags (csv)", tagsBuffer, sizeof(tagsBuffer)))
            {
                selectedModel->setGameplayTags(parseTagsCsv(tagsBuffer));
            }

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
                bool settingsChanged = false;

                int bodyTypeIndex = static_cast<int>(settings.bodyType);
                if (ImGui::Combo("Body Type", &bodyTypeIndex, kBodyTypeLabels, IM_ARRAYSIZE(kBodyTypeLabels)))
                {
                    settings.bodyType = static_cast<Physics::BodyType>(bodyTypeIndex);
                    settingsChanged = true;
                }

                int motionTypeIndex = static_cast<int>(settings.motionType);
                if (ImGui::Combo("Motion Type", &motionTypeIndex, kMotionTypeLabels, IM_ARRAYSIZE(kMotionTypeLabels)))
                {
                    settings.motionType = static_cast<Physics::MotionType>(motionTypeIndex);
                    settingsChanged = true;
                }

                auto& layerRegistry = Physics::PhysicsLayerRegistry::instance();
                const auto& layerNames = layerRegistry.getLayerNames();
                if (std::find(layerNames.begin(), layerNames.end(), settings.physicsLayer) == layerNames.end())
                {
                    settings.physicsLayer = layerNames.empty() ? "Default" : layerNames.front();
                    settingsChanged = true;
                }

                std::vector<const char*> layerNamePtrs;
                layerNamePtrs.reserve(layerNames.size());
                for (const auto& name : layerNames)
                {
                    layerNamePtrs.push_back(name.c_str());
                }

                int selectedLayerIndex = 0;
                for (size_t i = 0; i < layerNames.size(); ++i)
                {
                    if (layerNames[i] == settings.physicsLayer)
                    {
                        selectedLayerIndex = static_cast<int>(i);
                        break;
                    }
                }
                if (!layerNamePtrs.empty() && ImGui::Combo("Physics Layer", &selectedLayerIndex, layerNamePtrs.data(), static_cast<int>(layerNamePtrs.size())))
                {
                    settings.physicsLayer = layerNames[static_cast<size_t>(selectedLayerIndex)];
                    settingsChanged = true;
                }

                if (ImGui::Checkbox("Is Sensor (Trigger)", &settings.isSensor))
                {
                    settingsChanged = true;
                }

                if (settings.bodyType == Physics::BodyType::RigidBody)
                {
                    int colliderShapeIndex = static_cast<int>(settings.rigidBodyData.colliderShape);
                    if (ImGui::Combo("Collider Shape", &colliderShapeIndex, kColliderShapeLabels, IM_ARRAYSIZE(kColliderShapeLabels)))
                    {
                        settings.rigidBodyData.colliderShape = static_cast<Physics::ColliderShape>(colliderShapeIndex);
                        settingsChanged = true;
                    }

                    ImGui::SeparatorText("Transformation Offset");
                    settingsChanged |= ImGui::DragFloat3("Offset Position", &settings.rigidBodyData.transformationOffset.position.x, 0.05f);
                    settingsChanged |= ImGui::DragFloat3("Offset Rotation", &settings.rigidBodyData.transformationOffset.rotation.x, 0.5f);
                    settingsChanged |= ImGui::DragFloat3("Offset Scale", &settings.rigidBodyData.transformationOffset.scale.x, 0.05f, 0.01f, 1000.0f);

                    ImGui::SeparatorText("Dynamics");
                    if (ImGui::Checkbox("Override Mass", &settings.rigidBodyData.overrideMass))
                    {
                        settingsChanged = true;
                    }
                    if (settings.rigidBodyData.overrideMass)
                    {
                        settingsChanged |= ImGui::DragFloat("Mass", &settings.rigidBodyData.mass, 0.1f, 0.001f, 100000.0f);
                    }
                    settingsChanged |= ImGui::DragFloat3("Center of Mass Offset", &settings.rigidBodyData.centerOfMassOffset.x, 0.01f);

                    ImGui::SeparatorText("Surface Material");
                    settingsChanged |= ImGui::DragFloat("Friction", &settings.rigidBodyData.friction, 0.01f, 0.0f, 10.0f);
                    settingsChanged |= ImGui::DragFloat("Restitution", &settings.rigidBodyData.restitution, 0.01f, 0.0f, 1.0f);

                    ImGui::SeparatorText("Environmental Resistance");
                    settingsChanged |= ImGui::DragFloat("Linear Damping", &settings.rigidBodyData.linearDamping, 0.01f, 0.0f, 10.0f);
                    settingsChanged |= ImGui::DragFloat("Angular Damping", &settings.rigidBodyData.angularDamping, 0.01f, 0.0f, 10.0f);

                    const bool isTriangleMeshCollider = settings.rigidBodyData.colliderShape == Physics::ColliderShape::Custom;
                    const bool isConvexHullCollider = settings.rigidBodyData.colliderShape == Physics::ColliderShape::ConvexHull;
                    if (isTriangleMeshCollider || isConvexHullCollider)
                    {
                        if (isTriangleMeshCollider && settings.motionType != Physics::MotionType::Static)
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
                            if (isTriangleMeshCollider && settings.motionType != Physics::MotionType::Static)
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
                                    settingsChanged = true;
                                }
                            }
                        }

                        if (!colliderValidationMessage.empty())
                        {
                            ImGui::TextWrapped("%s", colliderValidationMessage.c_str());
                        }
                    }
                }

                if (settingsChanged)
                {
                    selectedModel->setPhysicsBodySettings(settings);
                }
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
