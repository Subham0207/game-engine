namespace ProjectAssets
{
    void RenderSaveAs{
            switch(fileTypeOperation)
            {
                case FileTypeOperation::saveLevelAs: {
                    InputText("##Filename", saveAsFileName);
                    if (ImGui::Button("Save"))
                    {
                        level.levelname = saveAsFileName;
                        Level::saveToFile("Assets/" + saveAsFileName, level);
                        showFileDialog = false;
                    }
                }
                break;

                case FileTypeOperation::saveModel: {
                    InputText("##Filename", saveAsFileName);
                    if (ImGui::Button("Save"))
                    {
                        auto model = mModels->at(selectedModelIndex);
                        Model::saveSerializedModel("Assets/" + saveAsFileName, *model);
                        //Recurrsively call save texture on the texture method ?
                        showFileDialog = false;
                    }
                }
                break;
            };
    }
}