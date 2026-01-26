#include <UI/outliner.hpp>
#include <UI/materialManager.hpp>
#include <UI/ProjectManager.hpp>
#include <UI/CharacterUI.hpp>
#include <UI/StatemachineUI.hpp>
#include <UI/PropertiesPanel.hpp>
#include <AI/AI.hpp>
#include <UI/AI_UI.hpp>

#include "UI/FileExplorer.hpp"

void Outliner::Render(Level &lvl) {
    if(ImGui::Begin("Outliner"))
    {

        if(ImGui::Button("Create a new Project"))
        {
            getUIState().createANewProject = true;
        }

        levelControlsComponent(lvl);

        coordinateSystemSelectorComponent();

        ImGui::NewLine();

        modelSelectorComponent();

        ImGui::NewLine();

        //Show transformation of the selected model
        ImGui::PushItemWidth(40);
        ModelMatrixComponent();

        manageModels();

        manageAnimationsForSelectedModel();

        debugOptions();

        popupForErrorsAndWarning();

        propertiesPanel();
    
        ImGui::End();
    }

    getUIState().characterUIState->draw();

    if(getUIState().blendspace2DUIState->showBlendspaceUI)
    {
        auto blendspace = getUIState().blendspace2DUIState->UIOpenedForBlendspace;
        blendspace->setScrubberLocation(
        glm::vec2( getUIState().blendspace2DUIState->scrubbedPoint.x,
        getUIState().blendspace2DUIState->scrubbedPoint.y));
        auto selection = blendspace->GetBlendSelection
        ();
        UI::Blendspace2DUI::draw(blendspace, selection, getUIState().blendspace2DUIState->showBlendspaceUI);
    }

    if(getUIState().statemachineUIState->showStateMachineUI)
    UI::StatemachineUI::draw(getUIState().statemachineUIState->UIOpenedForStatemachine, getUIState().statemachineUIState->showStateMachineUI);

    if(getUIState().ai_ui_state->showUI)
    getUIState().ai_ui_state->draw();
    
    if(getUIState().createANewProject)
    {
        ProjectAsset::createANewProject(
        getUIState().currentPath, EngineState::state->uiState.fileNames, getUIState().createANewProject 
        );
    }
    
    handlerForUIComponentsvisibility();

    if(getUIState().isFirstFrame){
    ImGui::SetWindowFocus(false);
    getUIState().isFirstFrame = false;
    }
}

void Outliner::ModelMatrixComponent()
{
            if(getUIState().selectedRenderableIndex > -1)
        {
            auto index = getUIState().selectedRenderableIndex;
            glm::mat4& modelMatrix = getActiveLevel().renderables.at(index)->getModelMatrix();
            glm::vec3 scale, rotation, translation, skew;
            glm::vec4 perspective;
            glm::quat orientation;
            
            //Decompose the selected model matrix into translation, rotation and scale vectors
            if (glm::decompose(modelMatrix, scale, orientation, translation, skew, perspective)) 
            {
                // Convert from radians to degrees if needed
                rotation = glm::degrees(glm::eulerAngles(orientation));

                ImGui::Text("Translation");
                ImGui::DragFloat("X##translation", &translation.x, 0.005f);
                ImGui::SameLine();
                ImGui::DragFloat("Y##translation", &translation.y, 0.005f);
                ImGui::SameLine();
                ImGui::DragFloat("Z##translation", &translation.z, 0.005f);

                // ImGui interface for scale
                ImGui::Text("Scale");
                ImGui::DragFloat("X##scale", &scale.x, 0.005f, 0.1f, 100.0f);  // Clamped to reasonable values
                ImGui::SameLine();
                if(scaleUniformly)
                {
                    scale.z = scale.y = scale.x;
                }
                else{
                    ImGui::DragFloat("Y##scale", &scale.y, 0.005f, 0.1f, 100.0f);
                    ImGui::SameLine();
                    ImGui::DragFloat("Z##scale", &scale.z, 0.005f, 0.1f, 100.0f);
                    ImGui::SameLine();
                }
                ImGui::Checkbox("Scale uniformly", &scaleUniformly);

                // // // ImGui interface for rotation
                ImGui::Text("Rotation");
                ImGui::DragFloat("X##rotation", &rotation.x, 0.1f);
                ImGui::SameLine();
                ImGui::DragFloat("Y##rotation", &rotation.y, 0.1f);
                ImGui::SameLine();
                ImGui::DragFloat("Z##rotation", &rotation.z, 0.1f);
                
                // glm::quat combinedRotation = orientation * newRotation;
                modelMatrix = glm::recompose(scale, glm::quat(glm::radians(rotation)), translation, skew, perspective);

            } else 
            {
                std::cerr << "Failed to decompose the the matrix" << std::endl;
            }

            //set radius and half height of capsule collider used on the character
            if(!EngineState::state->isPlay)
            {
                if(auto character = std::dynamic_pointer_cast<Character>(getActiveLevel().renderables.at(getUIState().selectedRenderableIndex)))
                {
                        auto capsule = character->capsuleCollider;

                        ImGui::Text("Capsule Collider Settings");

                        ImGui::PushItemWidth(150.0f);
                        ImGui::InputFloat("Radius", &capsule->radius, 0.1f, 0.25f, "%.3f");
                        ImGui::InputFloat("Half Height", &capsule->halfHeight, 0.1f, 0.25f, "%.3f");
                        ImGui::PopItemWidth();

                        // Clamp to avoid invalid geometry
                        capsule->radius = std::max(0.01f, capsule->radius);
                        capsule->halfHeight = std::max(0.01f, capsule->halfHeight);

                        if (ImGui::Button("Apply"))
                        {
                            capsule->reInit(capsule->radius, capsule->halfHeight);
                        }

                        ImGui::Text("Capsule Collider position");
                        ImGui::DragFloat("X1##translation", &character->capsuleColliderPosRelative.x, 0.005f);
                        ImGui::SameLine();
                        ImGui::DragFloat("Y1##translation", &character->capsuleColliderPosRelative.y, 0.005f);
                        ImGui::SameLine();
                        ImGui::DragFloat("Z1##translation", &character->capsuleColliderPosRelative.z, 0.005f);
                }
            }
            else
            {
                if(auto character = std::dynamic_pointer_cast<Character>(getActiveLevel().renderables.at(getUIState().selectedRenderableIndex)))
                {
                    ImGui::DragFloat("camera height", &character->cameraHeight, 0.005f);
                    ImGui::SameLine();
                    ImGui::DragFloat("camera distance", &character->cameraDistance, 0.005f);
                }
            }
        }
}
void Outliner::levelControlsComponent(Level &lvl)
{
    ImGui::Text("Current Level %s", lvl.levelname.c_str());
    if(ImGui::Button("Save Level"))
    {
        auto levelPath = EngineState::navIntoProjectDir("Levels");
        lvl.save(levelPath);
    }
    if(ImGui::Button("Save Level as"))
    {
        //POP up another window to give the lvl file a name and save perhaps ?
        //give level a name
        //choose the directory
        getUIState().saveAsFileName = "";
        getUIState().selectAFile = true;
        getUIState().showOpenButton = false;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::saveLevelAs;
    }
    if(ImGui::Button("Load a level"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileExtension = ".lvl";
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::LoadLvlFile;
    }

    if(ImGui::Button("Create character"))
    {
        auto character = new CharacterPrefabConfig();
        getUIState().characterUIState->start(*character);
    }

    if(ImGui::Button("Create new blendspace"))
    {
        auto blendspace = new BlendSpace2D();
        getUIState().blendspace2DUIState->UIOpenedForBlendspace = blendspace;
        getUIState().blendspace2DUIState->showBlendspaceUI = true;
    }

    if(ImGui::Button("Create new statmachine"))
    {
        UI::StatemachineUI::start();
    }

    if(ImGui::Button("Create new AI"))
    {
        UI::AI_UI::start();
    }
    
    std::vector<const char*> cPlayerControllerNames;
    for (const auto& pc : EngineState::state->playerControllers) {
    cPlayerControllerNames.push_back(pc->filename.c_str());
    }

    if (ImGui::Combo("Select Player controller to posses",
         &EngineState::state->activePlayerControllerId,
          cPlayerControllerNames.data(),
           cPlayerControllerNames.size())) {
    }
    
    ImGui::Text("--------AIs-------------");
        for (const auto ai : getActiveLevel().AIs) {
            ImGui::Text(ai->contentName().c_str());
        }
    ImGui::Text("------------------------");


    ImGui::Text("Navmesh debug");
    ImGui::Text("Starting Location");
    ImGui::DragFloat("x##startingLocation", &getUIState().startingLoc.x, 0.005f);
    ImGui::DragFloat("y##startingLocation", &getUIState().startingLoc.y, 0.005f);
    ImGui::DragFloat("z##startingLocation", &getUIState().startingLoc.z, 0.005f);

    ImGui::Text("Target location");
    ImGui::DragFloat("x##targetLocation", &getUIState().targetLoc.x, 0.005f);
    ImGui::DragFloat("y##targetLocation", &getUIState().targetLoc.y, 0.005f);
    ImGui::DragFloat("z##targetLocation", &getUIState().targetLoc.z, 0.005f);

    if(ImGui::Button("FindPath"))
    {
        getUIState().ai->calculatePath(getUIState().startingLoc, getUIState().targetLoc);
    }
}
void Outliner::modelSelectorComponent()
{
    for (int i = 0; i < getActiveLevel().renderables.size(); ++i) {
        // Optionally, push style changes here if you want to customize appearance
        // Render the radio button
        // The label for each button could be customized further if needed
        std::string name = getActiveLevel().renderables.at(i)->getName();
        name+=std::to_string(i);
        if (ImGui::RadioButton(name.c_str(), &getUIState().selectedRenderableIndex, i)) {
            if(getUIState().selectedRenderableIndex == i)
            {
                getActiveLevel().renderables.at(i)->setIsSelected(true);
            }
            else
            {
                getActiveLevel().renderables.at(i)->setIsSelected(false);
                
            }
        }
        // Optionally, pop style changes here if you made any
    }
}
void Outliner::coordinateSystemSelectorComponent()
{
    ImGui::Text("Coordinate space");
    if (ImGui::RadioButton("World space",&getUIState().coordinateSpace,0)) {
        EngineState::state->isWorldSpace = true;
    }
    if (ImGui::RadioButton("Local space",&getUIState().coordinateSpace,1)) {
        EngineState::state->isWorldSpace = true;
    }
}
void Outliner::manageModels()
{
    if(ImGui::Button("Import a Model"))
    {
        getUIState().loadModelWindow = true;
    }
    if(ImGui::Button("Save a Model"))
    {
        //Get selected index of the model
        //open UI to enter name of the model
        //call saveModel function
        getUIState().selectAFile = true;
        getUIState().showOpenButton = false;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::saveModel;
    }
    if(ImGui::Button("Load a Model"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::loadModel;
    }
}
void Outliner::manageAnimationsForSelectedModel()
{
    if(ImGui::Button("Play"))
    {
        EngineState::state->isPlay = true;
    }
    if(ImGui::Button("End Play"))
    {
        EngineState::state->isPlay = false;
    }

    if(ImGui::Button("import an animation"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::importAnimation;
    }
    std::vector<const char*> cStringAnimationNames;
    for (const auto& name : getUIState().animationNames) {
        cStringAnimationNames.push_back(name.c_str());
    }

    if (ImGui::Combo("Choose an option", &getUIState().selectedAnimationIndex, cStringAnimationNames.data(), cStringAnimationNames.size())) {
        // Action when the selection changes
        // std::cout << "Selected: " << animationNames[current_item] << std::endl;
    }
    if(ImGui::Button("Play Animation") && !EngineState::state->isPlay)
    {
        if(getUIState().selectedRenderableIndex > -1)
        {
            if(auto character = dynamic_cast<Character *>(getUIState().renderables[getUIState().selectedRenderableIndex]))
            {
                std::cout << "Character: "<< character->getName() << std::endl;
                character->animator->PlayAnimation(getUIState().animations[getUIState().selectedAnimationIndex]);
            }
        }
        // we need to send the manipulation to mesh
    }
        if(ImGui::Button("Stop Animation"))
    {
        if(auto character = dynamic_cast<Character *>(getUIState().renderables[getUIState().selectedRenderableIndex]))
        {
            character->animator->isAnimationPlaying = false;
        }
        // we need to send the manipulation to mesh
    }
}
void Outliner::popupForErrorsAndWarning()
{
    if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(EngineState::state->errorStack.LastElement().c_str());
        ImGui::Separator();

        if (ImGui::Button("OK")) { 
            EngineState::state->errorStack.LastElement() = "";
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(EngineState::state->errorStack.size() != 0)
    ImGui::OpenPopup("Warning");
}
void Outliner::debugOptions()
{

    if(EngineState::state->playerControllers.size() > 0)
    {
        auto playerController = EngineState::state->playerControllers[0];
        // if(playerController)
        // {
        //     ImGui::SliderFloat("X blendFactor", &playerController->movementDirection, -2.0f, -4.0f);
        //     ImGui::SliderFloat("Y blendFactor", &playerController->movementSpeed, -2.0f, -4.0f);
        // }
    }

    // ImGui::SliderFloat("X blendFactor", &getUIState().xblendFactor, 0.0f, 2.0f);
    // ImGui::SliderFloat("Y blendFactor", &getUIState().yblendFactor, 0.0f, 2.0f);


    if(ImGui::Button("Toggle Render bones"))
    {
        getUIState().showDebugBone = !getUIState().showDebugBone;
    }

    auto selectedIndex = 0;
    if(getUIState().selectedRenderableIndex > -1)
    {
        selectedIndex = getUIState().selectedRenderableIndex;
    }
    ImGui::Text("Selected bone");

    ImGui::SameLine();
    ImGui::InputInt("##number", &getUIState().selectedBoneId, 1);


    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        getUIState().selectedBoneId = -1;
    }

    ImGui::SameLine();
    ImGui::Text("Current number: %d", getUIState().selectedBoneId);
}

void Outliner::handlerForUIComponentsvisibility()
{
    if(getUIState().selectAFile)
    {
        ProjectAsset::selectOrLoadAFileFromFileExplorer(getUIState().currentPath, EngineState::state->uiState.fileNames, getUIState().selectAFile);
    }
    else if(getUIState().saveAFile){
        ProjectAsset::saveAFile(getUIState().currentPath, EngineState::state->uiState.fileNames, getUIState().saveAFile);
    }

    if(getUIState().loadModelWindow)
    {
        ModelAndTextureSelectionWindow();
    }
}
void Outliner::ModelAndTextureSelectionWindow()
{
    
    if(ImGui::Begin("Import Model", &getUIState().loadModelWindow))
    {   
        // getUIState().selectAFile = false;
        // getUIState().materials.clear();
        
        ImGui::Text("Model");
        ImGui::SameLine();
        ImGui::Text("%s", getUIState().modelfileName.c_str());
        ImGui::SameLine();
        if(ImGui::Button("Browse Model##1"))
        {
            getUIState().selectAFile = true;
            getUIState().showOpenButton = true;
            getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::importModelFile;
        }

        UI::renderMaterialManagerComponent();

        if(ImGui::Button("Done##1"))
        {
            getUIState().materials.clear();
            getUIState().loadModelWindow = false;
        }

        if(ImGui::Button("Save"))
        {
            auto dir = fs::path(EngineState::state->currentActiveProjectDirectory) / "Assets";
            if(auto character = std::dynamic_pointer_cast<Character>(getActiveLevel().renderables.at(getUIState().selectedRenderableIndex)))
            {
                character->save(dir);
            }

            if(auto model = std::dynamic_pointer_cast<Model>(getActiveLevel().renderables.at(getUIState().selectedRenderableIndex)))
            {
                model->save(dir);
            }
        }

        ImGui::End();
    }
}

void Outliner::applyRotation(glm::mat4& modelMatrix, glm::vec3 rotationDegrees, bool isLocal)
{
    // Convert degrees to radians for rotation
    glm::vec3 rotationRadians = glm::radians(rotationDegrees);

    // Prepare rotation quaternions based on the input
    glm::quat quatX = glm::angleAxis(rotationRadians.x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat quatY = glm::angleAxis(rotationRadians.y, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat quatZ = glm::angleAxis(rotationRadians.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Combine rotations
    glm::quat combinedQuat = quatZ * quatY * quatX;

    // Calculate rotation matrix
    glm::mat4 rotationMatrix = glm::mat4_cast(combinedQuat);

    // Apply rotation
    if (isLocal) {
        // Local rotation
        modelMatrix *= rotationMatrix;
    } else {
        // World rotation: rotate around global axes
        // Reset the translation part to avoid moving the object
        glm::vec3 translation = glm::vec3(modelMatrix[3]);
        modelMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        modelMatrix = rotationMatrix * modelMatrix;
        modelMatrix[3] = glm::vec4(translation, 1.0f);
    }
}

void Outliner::propertiesPanel()
{
    getUIState().propretiesPanel->draw();
}