namespace ProjectAssets{
    void RenderOpenFile(){
        ImGui::Text("Selected File: %s", filePath.c_str());
            if (ImGui::Button("Open"))
            {
                if (!fs::is_directory(filePath))
                {
                    switch (fileTypeOperation)
                    {
                        case FileTypeOperation::LoadLvlFile:
                            {
                                Level::loadFromFile(filePath, level);
                                showFileDialog = false;
                            }
                            break;
                        case FileTypeOperation::importModelFile:
                            {
                                modelfileName = filePath;
                                character = new Character();
                                character->model = new Model(const_cast<char*>(filePath.c_str()));
                                level.addModel(character->model);    
                                showFileDialog = false;                       
                            }
                            break;
                        case FileTypeOperation::albedoTexture:
                            {
                                albedo = filePath;
                                character->model->LoadTexture(filePath, aiTextureType_DIFFUSE);
                                showFileDialog = false;                      
                            }
                            break;
                        case FileTypeOperation::normalTexture:
                            {
                                normal = filePath;
                                character->model->LoadTexture(filePath, aiTextureType_NORMALS);
                                showFileDialog = false;                      
                            }
                            break;
                        case FileTypeOperation::metalnessTexture:
                            {
                                metalness = filePath;
                                character->model->LoadTexture(filePath, aiTextureType_METALNESS);
                                showFileDialog = false;                      
                            }
                            break;
                        case FileTypeOperation::roughnessTexture:
                            {
                                roughness = filePath;
                                character->model->LoadTexture(filePath, aiTextureType_DIFFUSE_ROUGHNESS);
                                showFileDialog = false;                      
                            }
                            break;
                        case FileTypeOperation::aoTexture:
                            {
                                ao = filePath;
                                character->model->LoadTexture(filePath, aiTextureType_AMBIENT_OCCLUSION);
                                showFileDialog = false;                      
                            }
                            break;
                        case FileTypeOperation::loadModel:
                            {
                                character->model = new Model();
                                Model::loadFromFile(filePath, *character->model);
                                level.addModel(character->model);
                                showFileDialog = false;                      
                            }
                            break;
                        case FileTypeOperation::loadAnimation:
                            {
                                if(character != NULL)
                                {
                                    auto animation = new Animation(filePath, character->model);
                                    animations.push_back(animation);
                                    animationNames.push_back(animation->animationName);
                                }
                                else
                                {
                                    errorMessage = "Please first Load a model and have it selected before loading an animation";
                                }
                                showFileDialog = false;                      
                            }
                            break;
                    
                    default:
                        break;
                    }
                }
            }
    }
}