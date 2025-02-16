#include <UI/outliner.hpp>
#include <UI/materialManager.hpp>
 
void Outliner::ModelMatrixComponent()
{
            if(getUIState().selectedRenderableIndex > -1)
        {
            glm::mat4& modelMatrix = (getUIState().renderables)[getUIState().selectedRenderableIndex]->getModelMatrix();
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
        }
}
void Outliner::levelControlsComponent(Level &lvl)
{
    ImGui::Text("Current Level %s", lvl.levelname.c_str());
    if(ImGui::Button("Save Level"))
    {
        Level::saveToFile(lvl.levelname, lvl);
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
}
void Outliner::modelSelectorComponent()
{
    for (int i = 0; i < getUIState().renderables.size(); ++i) {
        // Optionally, push style changes here if you want to customize appearance
        // Render the radio button
        // The label for each button could be customized further if needed
        std::string name = (getUIState().renderables)[i]->getName();
        name+=std::to_string(i);
        if (ImGui::RadioButton(name.c_str(), &getUIState().selectedRenderableIndex, i)) {
        }
        // Optionally, pop style changes here if you made any
    }
}
void Outliner::coordinateSystemSelectorComponent()
{
    ImGui::Text("Coordinate space");
    if (ImGui::RadioButton("World space",&getUIState().coordinateSpace,0)) {
        State::state->isWorldSpace = true;
    }
    if (ImGui::RadioButton("Local space",&getUIState().coordinateSpace,1)) {
        State::state->isWorldSpace = true;
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
    if(ImGui::Button("Load animation for model"))
    {
        getUIState().selectAFile = true;
        getUIState().showOpenButton = true;
        getUIState().fileTypeOperation = ProjectAsset::FileTypeOperation::loadAnimation;
    }
    std::vector<const char*> cStringAnimationNames;
    for (const auto& name : getUIState().animationNames) {
        cStringAnimationNames.push_back(name.c_str());
    }

    if (ImGui::Combo("Choose an option", &getUIState().selectedAnimationIndex, cStringAnimationNames.data(), cStringAnimationNames.size())) {
        // Action when the selection changes
        // std::cout << "Selected: " << animationNames[current_item] << std::endl;
    }
    if(ImGui::Button("Play Animation"))
    {
        if(auto character = dynamic_cast<Character *>(getUIState().renderables[getUIState().selectedRenderableIndex]))
        {
            std::cout << "Character: "<< character->getName() << std::endl;
            character->animator->PlayAnimation(getUIState().animations[getUIState().selectedAnimationIndex]);
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
        ImGui::Text(State::state->errorStack.LastElement().c_str());
        ImGui::Separator();

        if (ImGui::Button("OK")) { 
            State::state->errorStack.LastElement() = "";
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if(State::state->errorStack.size() != 0)
    ImGui::OpenPopup("Warning");
}
void Outliner::debugOptions()
{
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
        ProjectAsset::selectOrLoadAFileFromFileExplorer(getUIState().currentPath, State::state->uiState.fileNames, getUIState().selectAFile);
    }
    else if(getUIState().saveAFile){
        ProjectAsset::saveAFile(getUIState().currentPath, State::state->uiState.fileNames, getUIState().saveAFile);
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
        if(ImGui::Button("Browse##1"))
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
